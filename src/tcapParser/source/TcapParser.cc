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
//
// Created by barnett on 10/25/16.
//

#include "TcapParser.h"

#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "TcapLexer.h"

using std::function;
using std::make_shared;
using std::shared_ptr;
using std::string;
using std::vector;

using pdb_detail::TokenStream;
using pdb_detail::lexTcap;

namespace pdb_detail
{
    shared_ptr<Token> consumeNext(TokenStream &tokenStream, int expectedType1, int expectedType2)
    {
        if(!tokenStream.hasNext())
            return nullptr;

        Token next = tokenStream.advance();

        if(next.tokenType == expectedType1 || next.tokenType == expectedType2)
            return make_shared<Token>(next);

        return nullptr;

    }

    shared_ptr<Token> consumeNext(TokenStream &tokenStream, int expectedType)
    {
        return consumeNext(tokenStream, expectedType, expectedType);
    }


    Identifier::Identifier(string contents) : contents(make_shared<string>(contents))
    {
    }

    Identifier::Identifier(shared_ptr<string> contents) : contents(contents)
    {

    }


    Attribute::Attribute(Identifier name,  shared_ptr<string> value) : name(name), value(value)
    {

    }



    Statement::Statement() : attributes(make_shared<vector<Attribute>>())
    {

    }


    StringLiteral::StringLiteral(string contents) : contents(make_shared<string>(contents))
    {
    }

    StringLiteral::StringLiteral(shared_ptr<string> contents) : contents(contents)
    {

    }


    bool RetainAllClause::isAll()
    {
        return true;
    }

    bool RetainAllClause::isNone()
    {
        return false;
    }

    void RetainAllClause::execute(function<void(RetainAllClause&)> forAll, function<void(RetainExplicitClause&)>, function<void(RetainNoneClause&)>)
    {
        forAll(*this);
    }

    bool RetainNoneClause::isAll()
    {
        return false;
    }

    bool RetainNoneClause::isNone()
    {
        return true;
    }

    void RetainNoneClause::execute(function<void(RetainAllClause&)> forAll, function<void(RetainExplicitClause&)>, function<void(RetainNoneClause&)> forNone)
    {
        forNone(*this);
    }



    RetainExplicitClause::RetainExplicitClause(shared_ptr<vector<Identifier>> columns) : columns(columns)
    {
    }

    RetainExplicitClause::RetainExplicitClause(Identifier column)
    {
        columns = make_shared<vector<Identifier>>();
        columns->push_back(column);
    }

    bool RetainExplicitClause::isAll()
    {
        return false;
    }

    bool RetainExplicitClause::isNone()
    {
        return false;
    }

    void RetainExplicitClause::execute(function<void(RetainAllClause&)>, function<void(RetainExplicitClause&)> forExplicit, function<void(RetainNoneClause&)>)
    {
        forExplicit(*this);
    }



    ApplyOperation::ApplyOperation(string applyTarget, ApplyOperationType applyType, Identifier inputTable, shared_ptr<vector<Identifier>> inputTableColumnNames,
                   shared_ptr<RetainClause> retain)
            : ApplyOperation(make_shared<string>(applyTarget), applyType, inputTable, inputTableColumnNames, retain)
    {
    }

    ApplyOperation::ApplyOperation(shared_ptr<string> applyTarget, ApplyOperationType applyType, Identifier inputTable,
                   shared_ptr<vector<Identifier>> inputTableColumnNames, shared_ptr<RetainClause> retain)
            : applyTarget(applyTarget), applyType(applyType),  inputTable(inputTable), inputTableColumnNames(inputTableColumnNames),
              retain(retain)
    {
    }

    void ApplyOperation::execute(function<void(LoadOperation&)>, function<void(ApplyOperation&)> forApply, function<void(FilterOperation&)>,
                 function<void(HoistOperation&)>, function<void(BinaryOperation&)>)
    {
        forApply(*this);
    }

    LoadOperation::LoadOperation(shared_ptr<string> source) : source(source)
    {

    }

    LoadOperation::LoadOperation(string source) : source(make_shared<string>(source))
    {

    }

    void LoadOperation::execute(function<void(LoadOperation&)> forLoad, function<void(ApplyOperation&)>,
                 function<void(FilterOperation&)>,function<void(HoistOperation&)> forHoist,
                 function<void(BinaryOperation&)> forBinaryOp)
    {
        return forLoad(*this);
    }


    StoreOperation::StoreOperation(Identifier outputTable, shared_ptr<vector<Identifier>> columnsToStore, shared_ptr<string> destination)
            : outputTable(outputTable), columnsToStore(columnsToStore),  destination(destination)
    {

    }

    StoreOperation::StoreOperation(Identifier outputTable,  shared_ptr<vector<Identifier>> columnsToStore, string destination)
            : outputTable(outputTable), columnsToStore(columnsToStore), destination(make_shared<string>(destination))
    {

    }

    void StoreOperation::execute(function<void(TableAssignment&)>, function<void(StoreOperation&)> forStore)
    {
        forStore(*this);
    }



    FilterOperation::FilterOperation(Identifier inputTableName, Identifier filterColumnName, shared_ptr<RetainClause> retain)
            : inputTableName(inputTableName), filterColumnName(filterColumnName), retain(retain)
    {

    }

    void FilterOperation::execute(function<void(LoadOperation&)>, function<void(ApplyOperation&)>,
                 function<void(FilterOperation&)> forFilter, function<void(HoistOperation&)> forHoist,
                 function<void(BinaryOperation&)> forBinaryOp)
    {
        return forFilter(*this);
    }

    shared_ptr<Identifier> makeIdentifier(TokenStream &tokens)
    {
        shared_ptr<Token> ident = consumeNext(tokens, TokenType::IDENTIFIER_TYPE);

        if(ident == nullptr)
            return nullptr;

        return make_shared<Identifier>(ident->value);
    }

    shared_ptr<vector<Identifier>> makeIdentList(TokenStream &tokens)
    {
        shared_ptr<Identifier> ident = makeIdentifier(tokens);

        if(ident == nullptr)
            return nullptr;

        shared_ptr<vector<Identifier>> idents = make_shared<vector<Identifier>>();

        idents->push_back(*ident.get());


        while(tokens.hasNext() && tokens.peek().tokenType == TokenType::COMMA_TYPE)
        {
            tokens.advance();

            ident = makeIdentifier(tokens);

            if(ident == nullptr)
                return nullptr;

            idents->push_back(*ident.get());
        }

        return idents;
    }


    TableAssignment::TableAssignment(Identifier tableName, shared_ptr<vector<Identifier>> columnNames,  shared_ptr<TableExpression> value)
            : tableName(tableName), columnNames(columnNames), value(value)
    {
    }

    void TableAssignment::execute(function<void(TableAssignment&)> forTableAssignment, function<void(StoreOperation&)>)
    {
        forTableAssignment(*this);
    }




    shared_ptr<RetainClause> makeRetainClause(TokenStream &tokens)
    {
        shared_ptr<Token> retain = consumeNext(tokens, TokenType::RETAIN_TYPE);

        if (retain == nullptr)
            return nullptr;

        if (tokens.peek().tokenType == TokenType::ALL_TYPE)
        {
            tokens.advance();
            return make_shared<RetainAllClause>();
        }

        if (tokens.peek().tokenType == TokenType::NONE_TYPE)
        {
            tokens.advance();
            return make_shared<RetainNoneClause>();
        }


        shared_ptr<vector<Identifier>> columns = makeIdentList(tokens);

        if(columns == nullptr)
            return nullptr;

        return make_shared<RetainExplicitClause>(columns);

        return nullptr;
    }


    HoistOperation::HoistOperation(string hoistTarget, Identifier inputTable, shared_ptr<vector<Identifier>> inputTableColumnNames,
                   shared_ptr<RetainClause> retain)
            : HoistOperation(make_shared<string>(hoistTarget), inputTable, inputTableColumnNames, retain)
    {
    }

    HoistOperation::HoistOperation(shared_ptr<string> hoistTarget, Identifier inputTable,
                   shared_ptr<vector<Identifier>> inputTableColumnNames, shared_ptr<RetainClause> retain)
            : hoistTarget(hoistTarget), inputTable(inputTable), inputTableColumnNames(inputTableColumnNames),
              retain(retain)
    {
    }

    void HoistOperation::execute(function<void(LoadOperation&)>, function<void(ApplyOperation&)>, function<void(FilterOperation&)>,
                 function<void(HoistOperation&)> forHoist, function<void(BinaryOperation&)> forBinaryOp)
    {
        return forHoist(*this);
    }


    shared_ptr<ApplyOperation> makeApplyOperation(TokenStream &tokens)
    {
        shared_ptr<Token> apply = consumeNext(tokens, TokenType::APPLY_TYPE);

        if(apply == nullptr)
            return nullptr;

        shared_ptr<Token> applyTargetType = consumeNext(tokens, TokenType::FUNC_TYPE, TokenType::METHOD_TYPE);

        if(applyTargetType == nullptr)
            return nullptr;

        ApplyOperationType applyType = applyTargetType->tokenType == TokenType::FUNC_TYPE ? ApplyOperationType::func : ApplyOperationType::method;


        shared_ptr<Token> applyTarget = consumeNext(tokens, TokenType::STRING_LITERAL_TYPE);

        if(applyTarget == nullptr)
            return nullptr;

        shared_ptr<Token> to = consumeNext(tokens, TokenType::TO_TYPE);

        if(to == nullptr)
            return nullptr;

        shared_ptr<Identifier> inputTableName = makeIdentifier(tokens);

        if(inputTableName == nullptr)
            return nullptr;

        shared_ptr<Token> lbracket = consumeNext(tokens, TokenType::LBRACKET_TYPE);

        if(lbracket == nullptr)
            return nullptr;

        shared_ptr<vector<Identifier>> inputTableInputColumnNames = makeIdentList(tokens);

        if(inputTableInputColumnNames == nullptr)
            return nullptr;

        shared_ptr<Token> rbracket = consumeNext(tokens, TokenType::RBRACKET_TYPE);

        if(rbracket == nullptr)
            return nullptr;

        shared_ptr<RetainClause> retainClause = makeRetainClause(tokens);

        return make_shared<ApplyOperation>(applyTarget->value, applyType, *inputTableName.get(), inputTableInputColumnNames, retainClause);


    }

    shared_ptr<HoistOperation> makeHoistOperation(TokenStream &tokens)
    {
        shared_ptr<Token> hoist = consumeNext(tokens, TokenType::HOIST_TYPE);

        if(hoist == nullptr)
            return nullptr;

        shared_ptr<Token> hoistTarget = consumeNext(tokens, TokenType::STRING_LITERAL_TYPE);

        if(hoistTarget == nullptr)
            return nullptr;

        shared_ptr<Token> from = consumeNext(tokens, TokenType::FROM_TYPE);

        if(from == nullptr)
            return nullptr;

        shared_ptr<Identifier> inputTableName = makeIdentifier(tokens);

        if(inputTableName == nullptr)
            return nullptr;

        shared_ptr<Token> lbracket = consumeNext(tokens, TokenType::LBRACKET_TYPE);

        if(lbracket == nullptr)
            return nullptr;

        shared_ptr<vector<Identifier>> inputTableInputColumnNames = makeIdentList(tokens);

        if(inputTableInputColumnNames == nullptr)
            return nullptr;

        shared_ptr<Token> rbracket = consumeNext(tokens, TokenType::RBRACKET_TYPE);

        if(rbracket == nullptr)
            return nullptr;

        shared_ptr<RetainClause> retainClause = makeRetainClause(tokens);

        return make_shared<HoistOperation>(hoistTarget->value, *inputTableName.get(), inputTableInputColumnNames, retainClause);


    }

    shared_ptr<LoadOperation> makeLoadOperation(TokenStream &tokens)
    {
        shared_ptr<Token> load = consumeNext(tokens, TokenType::LOAD_TYPE);

        if(load == nullptr)
            return nullptr;

        shared_ptr<Token> externSourceString = consumeNext(tokens, TokenType::STRING_LITERAL_TYPE);

        if(externSourceString == nullptr)
            return nullptr;

        return make_shared<LoadOperation>(externSourceString->value);

    }

    shared_ptr<StoreOperation> makeStoreOperation(TokenStream &tokens)
    {
        shared_ptr<Token> store = consumeNext(tokens, TokenType::STORE_TYPE);

        if(store == nullptr)
            return nullptr;

        shared_ptr<Identifier> outputTable = makeIdentifier(tokens);

        if(outputTable == nullptr)
            return nullptr;

        shared_ptr<Token> lbracket = consumeNext(tokens, TokenType::LBRACKET_TYPE);

        if(lbracket == nullptr)
            return nullptr;

        shared_ptr<vector<Identifier>> columnsToStore = makeIdentList(tokens);

        if(columnsToStore == nullptr)
            return nullptr;

        shared_ptr<Token> rbracket = consumeNext(tokens, TokenType::RBRACKET_TYPE);

        if(rbracket == nullptr)
            return nullptr;


        shared_ptr<Token> externSourceString = consumeNext(tokens, TokenType::STRING_LITERAL_TYPE);

        if(externSourceString == nullptr)
            return nullptr;

        return make_shared<StoreOperation>(*outputTable.get(), columnsToStore, externSourceString->value);

    }

    shared_ptr<FilterOperation> makeFilterOperation(TokenStream &tokens)
    {
        shared_ptr<Token> filter = consumeNext(tokens, TokenType::FILTER_TYPE);

        if(filter == nullptr)
            return nullptr;

        shared_ptr<Identifier> inputTableName = makeIdentifier(tokens);

        if(inputTableName == nullptr)
            return nullptr;

        shared_ptr<Token> by = consumeNext(tokens, TokenType::BY_TYPE);

        if(by == nullptr)
            return nullptr;


        shared_ptr<Identifier> filterColumnName = makeIdentifier(tokens);

        if(filterColumnName == nullptr)
            return nullptr;

        shared_ptr<RetainClause> retain = makeRetainClause(tokens);

        if(retain == nullptr)
            return nullptr;

        return make_shared<FilterOperation>(*inputTableName.get(), *filterColumnName.get(), retain);

    }


    void BinaryOperation::execute(function<void(LoadOperation&)>, function<void(ApplyOperation&)>,
                 function<void(FilterOperation&)>, function<void(HoistOperation&)>,
                 function<void(BinaryOperation&)> forBinaryOp)
    {
        forBinaryOp(*this);
    }



    GreaterThanOp::GreaterThanOp(Identifier lhsTableName, Identifier lhsColumnName, Identifier rhsTableName, Identifier rhsColumnName, shared_ptr<RetainClause> retain)
            : lhsTableName(lhsTableName), lhsColumnName(lhsColumnName), rhsTableName(rhsTableName), rhsColumnName(rhsColumnName), retain(retain)
    {

    }

    void GreaterThanOp::execute(function<void(GreaterThanOp&)> forGreaterThan)
    {
        forGreaterThan(*this);
    }


    shared_ptr<TableExpression> makeBinaryOperation(TokenStream &tokens)
    {
        shared_ptr<Identifier> lhsTableName = makeIdentifier(tokens);

        if(lhsTableName == nullptr)
            return nullptr;

        shared_ptr<Token> lbracket = consumeNext(tokens, TokenType::LBRACKET_TYPE);

        if(lbracket == nullptr)
            return nullptr;

        shared_ptr<Identifier> lhsColumnName = makeIdentifier(tokens);

        if(lhsColumnName == nullptr)
            return nullptr;

        shared_ptr<Token> rbracket = consumeNext(tokens, TokenType::RBRACKET_TYPE);

        if(rbracket == nullptr)
            return nullptr;


        shared_ptr<Token> op = consumeNext(tokens, TokenType::GREATER_THAN_TYPE);

        if(op == nullptr)
            return nullptr;

        shared_ptr<Identifier> rhsTableName = makeIdentifier(tokens);

        if(rhsTableName == nullptr)
            return nullptr;

        lbracket = consumeNext(tokens, TokenType::LBRACKET_TYPE);

        if(lbracket == nullptr)
            return nullptr;

        shared_ptr<Identifier> rhsColumnName = makeIdentifier(tokens);

        if(rhsColumnName == nullptr)
            return nullptr;

        rbracket = consumeNext(tokens, TokenType::RBRACKET_TYPE);

        if(rbracket == nullptr)
            return nullptr;

        shared_ptr<RetainClause> retain = makeRetainClause(tokens);

        if(retain == nullptr)
            return nullptr;

        if(op->tokenType == TokenType::GREATER_THAN_TYPE)
            return make_shared<GreaterThanOp>(*lhsTableName.get(), *lhsColumnName.get(), *rhsTableName.get(), *rhsColumnName.get(), retain);


        return nullptr;
    }

    shared_ptr<TableExpression> makeTableExpression(TokenStream &tokens)
    {
        switch(tokens.peek().tokenType)
        {
            case (TokenType::APPLY_TYPE):
                return makeApplyOperation(tokens);
            case (TokenType::HOIST_TYPE):
                return makeHoistOperation(tokens);
            case (TokenType::LOAD_TYPE):
                return makeLoadOperation(tokens);
            case(TokenType::FILTER_TYPE):
                return makeFilterOperation(tokens);
            default:
                return makeBinaryOperation(tokens);
        }
    }

    shared_ptr<TableAssignment> makeTableAssignment(TokenStream &tokens)
    {

        shared_ptr<Identifier> tableName = makeIdentifier(tokens);

        if(tableName == nullptr)
            return nullptr;

        shared_ptr<Token> lparen = consumeNext(tokens, TokenType::LPAREN_TYPE);

        if(lparen == nullptr)
            return nullptr;


        shared_ptr<vector<Identifier>> columnNames = makeIdentList(tokens);

        if(columnNames == nullptr)
            return nullptr;

        shared_ptr<Token> rparen = consumeNext(tokens, TokenType::RPAREN_TYPE);

        if(rparen == nullptr)
            return nullptr;

        shared_ptr<Token> eq = consumeNext(tokens, TokenType::EQ_TYPE);

        if(eq == nullptr)
            return nullptr;

        shared_ptr<TableExpression> tableExp = makeTableExpression(tokens);

        if(tableExp == nullptr)
            return nullptr;

        return make_shared<TableAssignment>(*tableName.get(), columnNames, tableExp);
    }

    shared_ptr<Attribute> makeAttribute(TokenStream &tokens)
    {
        shared_ptr<Token> at = consumeNext(tokens, TokenType::AT_SIGN_TYPE);

        if(at == nullptr)
            return nullptr;

        shared_ptr<Identifier> name = makeIdentifier(tokens);

        if(name == nullptr)
            return nullptr;

        shared_ptr<Token> value = consumeNext(tokens, TokenType::STRING_LITERAL_TYPE);

        if(value == nullptr)
            return nullptr;

        return make_shared<Attribute>(*name.get(), make_shared<string>(value->value));


    }

    shared_ptr<Statement> makeStatement(TokenStream &tokens)
    {
        shared_ptr<vector<Attribute>> attributes = make_shared<vector<Attribute>>();

        while(tokens.hasNext() && tokens.peek().tokenType == TokenType::AT_SIGN_TYPE)
        {
            shared_ptr<Attribute> attribute = makeAttribute(tokens);

            if(attribute == nullptr)
                return nullptr;

            attributes->push_back(*attribute.get());
        }

        shared_ptr<Statement> stmt;
        switch (tokens.peek().tokenType)
        {

            case TokenType::UNKNOWN_TYPE:
                return nullptr;
            case TokenType::STORE_TYPE:
                stmt = makeStoreOperation(tokens);
                break;
            default:
                stmt = makeTableAssignment(tokens);
                break;

        }

        if(stmt == nullptr)
            return nullptr;

        stmt->attributes = attributes;

        return stmt;

    }

    shared_ptr<TranslationUnit> parseTcap(const string &source)
    {
        TokenStream tokens = lexTcap(source);

        shared_ptr<TranslationUnit> unit = make_shared<TranslationUnit>();
        while(tokens.hasNext())
        {
            shared_ptr<Statement> stmt = makeStatement(tokens);
            if(stmt == nullptr)
                return nullptr;
            unit->statements->push_back(stmt);
        }

        return unit;
    }
}