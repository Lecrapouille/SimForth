//=====================================================================
// SimTaDyn: A GIS in a spreadsheet.
// Copyright 2018 Quentin Quadrat <lecrapouille@gmail.com>
//
// This file is part of SimTaDyn.
//
// SimTaDyn is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SimTaDyn.  If not, see <http://www.gnu.org/licenses/>.
//=====================================================================

#include "ForthInspector.hpp"
#include "Primitives.hpp"

static bool policy_inspect(forth::Token const *nfa,
                           forth::Dictionary const& dictionary,
                           ListStorePtr dicoModel,
                           ForthDicoInspector::ModelColumns& columns,
                           uint32_t const max_primitives)
{
    dicoModel->clear();
    auto row = *(dicoModel->append());
    row[columns.address] = nfa - dictionary();
    row[columns.name] = forth::NFA2Name(nfa);
    row[columns.immediate] = forth::isImmediate(nfa);
    row[columns.smudge] = forth::isSmudge(nfa);
    forth::Token code = *forth::NFA2CFA(nfa);
    row[columns.token] = code;
    if (code < max_primitives)
    {
        row[columns.definition] = "primitive";
    }
    else
    {
        row[columns.definition] = "todo";
    }
    return false;
}

void ForthDicoInspector::inspect(forth::Dictionary const& dictionary, uint32_t const max_primitives)
{
    forth::Token iter = dictionary.last();
    dictionary.iterate(policy_inspect, iter, 0, dictionary, m_dico_model, m_columns, max_primitives);
}
