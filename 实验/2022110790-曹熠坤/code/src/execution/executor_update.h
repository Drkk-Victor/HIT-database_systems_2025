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

class UpdateExecutor : public AbstractExecutor {
   private:
    TabMeta tab_;                        // 表的元数据   
    std::vector<Condition> conds_;       // unused
    RmFileHandle *fh_;                   //    
    std::vector<Rid> rids_;              // 要修改的记录
    std::string tab_name_;
    std::vector<SetClause> set_clauses_; // {{table, colomn}, value}
    SmManager *sm_manager_;

   public:
    UpdateExecutor(SmManager *sm_manager, const std::string &tab_name, std::vector<SetClause> set_clauses,
                   std::vector<Condition> conds, std::vector<Rid> rids, Context *context) {
        sm_manager_ = sm_manager;
        tab_name_ = tab_name;
        set_clauses_ = set_clauses;
        tab_ = sm_manager_->db_.get_table(tab_name);
        fh_ = sm_manager_->fhs_.at(tab_name).get();
        conds_ = conds;
        rids_ = rids; 
        context_ = context;
    }
    std::unique_ptr<RmRecord> Next() override {
        for (auto &rid : rids_) { // 对所有需要修改的记录
            auto rec = fh_->get_record(rid, context_);      // 从数据文件中获取记录
            for (auto &set_clause : set_clauses_) {
                auto lhs_col = tab_.get_col(set_clause.lhs.col_name); // 获取set字段名对应的字段元信息（位置、长度等）
                memcpy(rec->data + lhs_col->offset, set_clause.rhs.raw->data, lhs_col->len);    // 将新的值（set_clause.rhs.raw->data）复制到记录中相应列的位置
            }
            // Remove old index
            for (auto & index : tab_.indexes) {
                auto ih = sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name_, index.cols)).get();   // 获取索引的名称，并获得IndexHandle以操作该索引
                char *key = new char[index.col_tot_len]; // 索引键
                int offset = 0;
                for (size_t j = 0; j < index.col_num; j++) {
                    memcpy(key + offset, rec->data + index.cols[j].offset, index.cols[j].len);  // 拼接为一个完整的索引键
                    offset += index.cols[j].len;
                }
                ih->delete_entry(key, context_->txn_);  // 删除旧数据
            }
            // Update record in record file
            fh_->update_record(rid, rec->data, context_);   // 更新记录
            // Insert new index
            for (auto & index : tab_.indexes) {
                auto ih = sm_manager_->ihs_.at(sm_manager_->get_ix_manager()->get_index_name(tab_name_, index.cols)).get();
                char *key = new char[index.col_tot_len];
                int offset = 0;
                for (size_t j = 0; j < index.col_num; j++) {
                    memcpy(key + offset, rec->data + index.cols[j].offset, index.cols[j].len);
                    offset += index.cols[j].len;
                }
                ih->insert_entry(key, rid, context_->txn_); // 同理的在相应位置插入新数据
            }
        }
        return nullptr;
    }

    Rid &rid() override { return _abstract_rid; }
};