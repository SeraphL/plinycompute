/*****************************************************************************
 *                                                                           *
 *  Copyright 2018 Rice University                                           *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *      http://www.apache.org/licenses/LICENSE-2.0                           *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 *****************************************************************************/
#include "TcapIrBuilder.h"

#include "Store.h"

namespace pdb_detail
{
    string getAttributeValue(string name, shared_ptr<vector<Attribute>> attributes)
    {
        for(Attribute a : *attributes.get())
        {
            if(*a.name.contents.get() == name)
            {
                string s = *a.value.get();
                s.erase(std::remove(s.begin(), s.end(), '"'), s.end());
                return s;
            }
        }

        throw "no such element";
    }

    shared_ptr<Instruction> makeInstructionFromApply(ApplyOperation &applyOp, TableAssignment& tableAssignment)
    {
        string executorId = getAttributeValue("exec", tableAssignment.attributes);

        shared_ptr<vector<Column>> empty = make_shared<vector<Column>>();

        string inputTableName = *applyOp.inputTable.contents.get();

        // make tableColumns
        shared_ptr<vector<string>> columnNames = make_shared<vector<string>>();
        {
            for (Identifier i : *applyOp.inputTableColumnNames.get())
            {
                columnNames->push_back(*i.contents.get());
            }
        }
        TableColumns inputColumns(*applyOp.inputTable.contents.get(), columnNames);

        shared_ptr<vector<Column>> outputColumnsToCopy = make_shared<vector<Column>>();
        {
            for(std::vector<int>::size_type i = 0; i < tableAssignment.columnNames->size()-1; i++)
            {
                string columnId = *tableAssignment.columnNames->operator[](i).contents.get();
                outputColumnsToCopy->push_back(Column(inputTableName, columnId));
            }
        }

        string functionId = *applyOp.applyTarget.get();
        string outputTableName = *tableAssignment.tableName.contents.get();

        // outputcolumnName is the last value in tableAssignment.columnNames
        string outputcolumnName = *tableAssignment.columnNames->operator[](tableAssignment.columnNames->size()-1).contents.get();

        if(applyOp.applyType == ApplyOperationType::func)
            return make_shared<ApplyFunction>(executorId, functionId , outputTableName, outputcolumnName, inputColumns, outputColumnsToCopy);
        else
            return make_shared<ApplyMethod>(executorId, functionId , outputTableName, outputcolumnName, inputColumns, outputColumnsToCopy);

    }

    shared_ptr<Instruction> makeInstruction(TableAssignment& tableAssignment)
    {
        shared_ptr<Instruction> instruction;

        string assignmentLhsTableName = *tableAssignment.tableName.contents.get();

        tableAssignment.value->execute(
                [&](LoadOperation &load)
                {
                    string unquotedSource = *load.source.get();
                    unquotedSource = unquotedSource.substr(1, unquotedSource.size()-2); // remove surrounding quotes
                    instruction = make_shared<Load>(assignmentLhsTableName,
                                                    *tableAssignment.columnNames->operator[](0).contents.get(),
                                                    unquotedSource);
                },
                [&](ApplyOperation  &applyOp)
                {
                    instruction = makeInstructionFromApply(applyOp, tableAssignment);

                },
                [&](FilterOperation &filterOperation)
                {
                    string inputTableName = *filterOperation.inputTableName.contents.get();
                    shared_ptr<vector<Column>> outputColumnsToCopy = make_shared<vector<Column>>();
                    {
                        for(std::vector<int>::size_type i = 0; i < tableAssignment.columnNames->size(); i++)
                        {
                            string columnId = *tableAssignment.columnNames->operator[](i).contents.get();
                            outputColumnsToCopy->push_back(Column(inputTableName, columnId));
                        }
                    }

                    instruction = make_shared<Filter>(inputTableName, *filterOperation.filterColumnName.contents.get(),
                                                      assignmentLhsTableName, outputColumnsToCopy);

                },
                [&](HoistOperation &hoistOperation)
                {
                    string inputTableName = *hoistOperation.inputTable.contents.get();
                    shared_ptr<vector<Column>> outputColumnsToCopy = make_shared<vector<Column>>();
                    {
                        for(std::vector<int>::size_type i = 0; i < tableAssignment.columnNames->size()-1; i++)
                        {
                            string columnId = *tableAssignment.columnNames->operator[](i).contents.get();
                            outputColumnsToCopy->push_back(Column(inputTableName, columnId));
                        }
                    }

                    Column hoistColumn(*hoistOperation.inputTable.contents.get(),
                                       *hoistOperation.inputTableColumnNames->operator[](0).contents.get());

                    string executorId = getAttributeValue("exec", tableAssignment.attributes);

                    string outputcolumnName = *tableAssignment.columnNames->operator[](tableAssignment.columnNames->size()-1).contents.get();

                    Column outputColumn(assignmentLhsTableName, outputcolumnName);

                    instruction = make_shared<Hoist>(*hoistOperation.hoistTarget.get(), hoistColumn,
                                                     outputColumn, outputColumnsToCopy,
                                                     executorId);

                },
                [&](BinaryOperation &binaryOperation)
                {
                    binaryOperation.execute(
                            [&](GreaterThanOp& gt)
                            {
                                Column lhs(*gt.lhsTableName.contents.get(), *gt.lhsColumnName.contents.get());
                                Column rhs(*gt.rhsTableName.contents.get(), *gt.rhsColumnName.contents.get());

                                string inputTableName = lhs.tableId;
                                shared_ptr<vector<Column>> outputColumnsToCopy = make_shared<vector<Column>>();
                                {
                                    for(std::vector<int>::size_type i = 0; i < tableAssignment.columnNames->size()-1; i++)
                                    {
                                        string columnId = *tableAssignment.columnNames->operator[](i).contents.get();
                                        outputColumnsToCopy->push_back(Column(inputTableName, columnId));
                                    }
                                }

                                string executorId = getAttributeValue("exec", tableAssignment.attributes);

                                string outputcolumnName = *tableAssignment.columnNames->operator[](tableAssignment.columnNames->size()-1).contents.get();

                                Column outputColumn(assignmentLhsTableName, outputcolumnName);

                                instruction = make_shared<GreaterThan>(lhs, rhs, outputColumn, outputColumnsToCopy,
                                                                       executorId);
                            });
                });

        return instruction;
    }

    shared_ptr<Instruction> makeInstruction(StoreOperation& storeOperation)
    {
        string unquotedDest = *storeOperation.destination.get();
        unquotedDest = unquotedDest.substr(1, unquotedDest.size()-2); // remove surrounding quotes

        return make_shared<Store>(*storeOperation.outputTable.contents.get(), unquotedDest);
    }

    shared_ptr<Instruction> makeInstruction(shared_ptr<Statement> stmt)
    {
        shared_ptr<Instruction> translated = nullptr;
        stmt->execute([&](TableAssignment& tableAssignment)
                      {
                          translated = makeInstruction(tableAssignment);
                      },
                      [&](StoreOperation& storeOperation)
                      {
                          translated = makeInstruction(storeOperation);
                      });

        return translated;
    }


    shared_ptr<vector<shared_ptr<Instruction>>> buildTcapIr(shared_ptr<TranslationUnit> unit)
    {
        shared_ptr<vector<shared_ptr<Instruction>>> instructions = make_shared<vector<shared_ptr<Instruction>>>();

        for(shared_ptr<Statement> stmt : *unit->statements.get())
        {
            try
            {
                instructions->push_back(makeInstruction(stmt));
            }
            catch (...)
            {
                return nullptr;
            }
        }

        return instructions;
    }
}