/*
    Copyright (c) 2013 Boldizsár Lipka <lipkab@zoho.com>

    This file is part of the Apertium Visual Rule Editor.

    Apertium Visual Rule Editor is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Apertium Visual Rule Editor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Apertium Visual Rule Editor.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "filesystem.h"

#define STR_EXPAND(arg) #arg
#define STR(arg) STR_EXPAND(arg)

namespace fs
{

namespace
{

const QString datadir = STR(VISRULED_DATADIR);

const QString visualSchemaFilePath =  datadir + "/res/schema.xml";
const QString valueMapFilePath = datadir + "/res/lists.xml";
const QString templatesFilePath = datadir + "/res/templates.xml";
const QString templatesDirPath = datadir + "/res/templates";

}

const QString& visualSchemaFile()
{
    return visualSchemaFilePath;
}

const QString& valueMapFile()
{
    return valueMapFilePath;
}

const QString& templatesFile()
{
    return templatesFilePath;
}

const QString& templatesDir()
{
    return templatesDirPath;
}

}

#undef STR_EXPAND
#undef STR
