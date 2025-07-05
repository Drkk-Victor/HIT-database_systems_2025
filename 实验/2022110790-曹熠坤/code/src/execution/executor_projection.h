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

class ProjectionExecutor : public AbstractExecutor {
   private:
    std::unique_ptr<AbstractExecutor> prev_;        // 投影节点的儿子节点
    std::vector<ColMeta> cols_;                     // 需要投影的字段的列元信息
    size_t len_;                                    // 字段总长度
    std::vector<size_t> sel_idxs_;                  // 所选的字段在prev_->cols()中的下标

   public:
    ProjectionExecutor(std::unique_ptr<AbstractExecutor> prev, const std::vector<TabCol> &sel_cols) {
        prev_ = std::move(prev); // 子执行器

        size_t curr_offset = 0;
        auto &prev_cols = prev_->cols(); // 子执行器输出的列
        for (auto &sel_col : sel_cols) {
            // 获取每一个投影列在（子节点输出列）中的位置（下标），构成sel_idxs_数组
            auto pos = get_col(prev_cols, sel_col);
            sel_idxs_.push_back(pos - prev_cols.begin());
            // 获取投影列的列元信息，并重新设置偏移量，放到cols_数组里
            auto col = *pos;
            col.offset = curr_offset;
            curr_offset += col.len;
            cols_.push_back(col);
        }
        len_ = curr_offset;
    }

    void beginTuple() override { // 使用执行器prev_定位到子节点算子的第一条结果元组
        prev_->beginTuple();
        if (!prev_->is_end()) {
            _abstract_rid = prev_->rid();
        }
    }

    void nextTuple() override { // 使用执行器prev_定位到子节点算子的下一条结果元组
        assert(!is_end());
        prev_->nextTuple();
        if (!prev_->is_end()) {
            _abstract_rid = prev_->rid();
        }    
    }

    std::unique_ptr<RmRecord> Next() override {
        assert(!is_end());

        auto full_rec = prev_->Next();  // 从子执行器拿到完整记录
        if (full_rec == nullptr) return nullptr;

        auto projected_rec = std::make_unique<RmRecord>(len_); // 创建返回的投影元组
        for (size_t i = 0; i < sel_idxs_.size(); i++) {
            const auto &prev_col = prev_->cols()[sel_idxs_[i]]; // 投影前列的位置
            const auto &this_col = cols_[i]; // 投影后列的位置
            memcpy(projected_rec->data + this_col.offset, // 列的新位置
                  full_rec->data + prev_col.offset, // 列的原位置
                  prev_col.len); // 长度
        }
        return projected_rec;
    }

    Rid &rid() override { return _abstract_rid; }

    bool is_end() const override { return prev_->is_end(); }
    
    std::string getType() override { return "ProjectionExecutor"; }

    size_t tupleLen() const override { return len_; }

    const std::vector<ColMeta> &cols() const override { return cols_; }
};