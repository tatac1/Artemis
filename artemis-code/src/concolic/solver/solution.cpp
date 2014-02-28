/*
 * Copyright 2012 Aarhus University
 *
 * Licensed under the GNU General Public License, Version 3 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *          http://www.gnu.org/licenses/gpl-3.0.html
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>

#include <QDebug>

#include "solution.h"

#include "statistics/statsstorage.h"

namespace artemis
{

Solution::Solution(bool success, bool unsat, QString unsolvableReason) :
    mSuccess(success),
    mUnsat(unsat),
    mUnsolvableReason(unsolvableReason)
{
}

bool Solution::isSolved() const
{
    return mSuccess;
}

bool Solution::isUnsat() const
{
    return mUnsat;
}

void Solution::insertSymbol(QString symbol, Symbolvalue value)
{
    mSymbols.insert(symbol, value);
}

Symbolvalue Solution::findSymbol(QString symbol)
{
    QHash<QString, Symbolvalue>::iterator iter = mSymbols.find(symbol);

    if (iter == mSymbols.end()) {
        Symbolvalue result;
        result.found = false;

        return result;
    }

    return iter.value();
}

void Solution::toStatistics()
{
    QHash<QString, Symbolvalue>::iterator iter = mSymbols.begin();
    for (; iter != mSymbols.end(); iter++) {
        QString key = QString("Concolic::Solver::Constraint.") + iter.key();

        Symbolvalue value = iter.value();

        switch (value.kind) {
        case Symbolic::INT:
            Statistics::statistics()->set(key, value.u.integer);
            break;
        case Symbolic::BOOL:
            Statistics::statistics()->set(key, value.u.boolean);
            break;
        case Symbolic::STRING:
            Statistics::statistics()->set(key, value.string);
            break;
        default:
            std::cerr << "Unimplemented value type encountered" << std::endl;
            std::exit(1);
        }
    }
}

} // namespace artemis






