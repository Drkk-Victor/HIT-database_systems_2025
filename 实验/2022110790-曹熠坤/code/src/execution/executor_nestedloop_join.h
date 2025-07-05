/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once
#include "execution_defs.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "index/ix.h"
#include "system/sm.h"

class NestedLoopJoinExecutor : public AbstractExecutor {
   private:
    std::unique_ptr<AbstractExecutor> left_;    // 左儿子节点（需要join的表）
    std::unique_ptr<AbstractExecutor> right_;   // 右儿子节点（需要join的表）
    size_t len_;                                // join后获得的每条记录的长度
    std::vector<ColMeta> cols_;                 // join后获得的记录的字段

    std::vector<Condition> fed_conds_;          // join条件
    bool isend;

   public:
    NestedLoopJoinExecutor(std::unique_ptr<AbstractExecutor> left, std::unique_ptr<AbstractExecutor> right, 
                            std::vector<Condition> conds) {
        left_ = std::move(left);
        right_ = std::move(right);
        len_ = left_->tupleLen() + right_->tupleLen();
        cols_ = left_->cols();
        auto right_cols = right_->cols();
        for (auto &col : right_cols) {
            col.offset += left_->tupleLen();
        }

        cols_.insert(cols_.end(), right_cols.begin(), right_cols.end());
        isend = false;
        fed_conds_ = std::move(conds);

    }

    void beginTuple() override {
        // 左右子执行器都移到第一条元组
        left_->beginTuple();
        if (left_->is_end()) {
            return;
        }
        right_->beginTuple();

        // 往下移直到满足连接条件
        for (;
            !is_end() && !eval_conds(left_->Next().get(), right_->Next().get(), fed_conds_, cols_);
            move_to_next()) {}
    }

    void nextTuple() override { // 移动到下一条满足条件的元组
        assert(!is_end());
        
        for (move_to_next();
            !is_end() && !eval_conds(left_->Next().get(), right_->Next().get(), fed_conds_, cols_); 
            move_to_next()){}
    }

    std::unique_ptr<RmRecord> Next() override { // 连接左右子执行器锁定的元组并返回
        auto left_rec = left_->Next();
        auto right_rec = right_->Next();
        auto joint_rec = std::make_unique<RmRecord>(len_);
        memcpy(joint_rec->data, left_rec->data, left_->tupleLen());
        memcpy(joint_rec->data + left_->tupleLen(), right_rec->data, right_->tupleLen());
        return joint_rec;
    }

    Rid &rid() override { return _abstract_rid; }

    bool is_end() const override { return left_->is_end(); }
    
    std::string getType() override { return "NestedLoopJoinExecutor"; }

    size_t tupleLen() const override { return len_; }

    const std::vector<ColMeta> &cols() const override { return cols_; }

    private:
    bool eval_cond(const RmRecord *lhs_rec, const RmRecord *rhs_rec, const Condition &cond, const std::vector<ColMeta> &rec_cols) { // 修改
        auto lhs_col = get_col(rec_cols, cond.lhs_col); //获取左表条件的列元数据
        char *lhs = lhs_rec->data + lhs_col->offset;   //获得参与比较的左值
        char *rhs;
        ColType rhs_type;
        if (cond.is_rhs_val) {  //是常量则直接获取常量值并设置它的类型
            rhs_type = cond.rhs_val.type;
            rhs = cond.rhs_val.raw->data;   //常量右值直接获得
        } else {
            // rhs is a column
            auto rhs_col = get_col(rec_cols, cond.rhs_col);
            rhs_type = rhs_col->type;       //右表的字段位置相对于整个连接结果的记录布局
            rhs = rhs_rec->data + rhs_col->offset - left_->tupleLen();
        }
        assert(rhs_type == lhs_col->type);  //确保左右列的类型一致
        int cmp = ix_compare(lhs, rhs, rhs_type, lhs_col->len);
        if (cond.op == OP_EQ) {
            return cmp == 0;
        } else if (cond.op == OP_NE) {
            return cmp != 0;
        } else if (cond.op == OP_LT) {
            return cmp < 0;
        } else if (cond.op == OP_GT) {
            return cmp > 0;
        } else if (cond.op == OP_LE) {
            return cmp <= 0;
        } else if (cond.op == OP_GE) {
            return cmp >= 0;
        } else {
            throw InternalError("Unexpected op type");
        }
    }

    bool eval_conds(const RmRecord *lhs_rec, const RmRecord *rhs_rec, const std::vector<Condition> &conds, const std::vector<ColMeta> &rec_cols) {
        return std::all_of(conds.begin(), conds.end(),
            [&](const Condition &cond) { return eval_cond(lhs_rec, rhs_rec, cond, rec_cols); }
        );
    }

    void move_to_next() // 如果右表的记录遍历完，则开始遍历下一条左表记录，并重新从头开始遍历右表
    {
        right_->nextTuple();
        if (right_->is_end()) {
            left_->nextTuple();
            right_->beginTuple();
        }
    }
};