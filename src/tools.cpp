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

#include "tools.h"
#include "config.h"
#include <QDebug>

void ToolsManager::morph(const QString &text)
{
    if (!canLaunchNew()) {
        return;
    }

    curProc_ = new QProcess;
    curType_ = MORPH;
    connect(curProc_, SIGNAL(readyReadStandardOutput()), this, SLOT(outputReady()));
    connect(curProc_, SIGNAL(finished(int)), this, SLOT(finished(int)));

    curProc_->start(appConfig().ltProcPath(), QStringList() << slbin_ << "-z");
    curProc_->write(text.toLatin1());
    curProc_->write("\n");
    curProc_->putChar(0);
}

void ToolsManager::transfer(const QString &text)
{
    if (!canLaunchNew()) {
        return;
    }

    curProc_ = new QProcess;
    curType_ = TRANSFER;
    connect(curProc_, SIGNAL(readyReadStandardOutput()), this, SLOT(outputReady()));
    connect(curProc_, SIGNAL(finished(int)), this, SLOT(finished(int)));

    //FIXME
    QString cmd = "echo \"" + text + "\" |" + appConfig().apertiumTransferPath() + " " + rules_ + " " + rulebin_ + " " + bibin_;
    curProc_->start("/bin/bash", QStringList() << "-c" << cmd);
}

void ToolsManager::generate(const QString &text)
{
    if (!canLaunchNew()) {
        return;
    }

    curProc_ = new QProcess;
    curType_ = GENERATE;
    connect(curProc_, SIGNAL(readyReadStandardOutput()), this, SLOT(outputReady()));
    connect(curProc_, SIGNAL(finished(int)), this, SLOT(finished(int)));

    QString cmd = "echo \"" + text + "\" |" + appConfig().ltProcPath() + " -gz " + tlbin_;
    curProc_->start("/bin/bash", QStringList() << "-c" << cmd);
}

void ToolsManager::compile()
{
    if (!canLaunchNew()) {
        return;
    }

    curProc_ = new QProcess(this);
    curType_ = COMPILE1;
    connect(curProc_, SIGNAL(finished(int)), this, SLOT(finished(int)));

    QStringList args;
    args.append("lr");
    args.append(sldict_);
    args.append(slbin_);

    curProc_->start(appConfig().ltCompPath(), args);
}

void ToolsManager::compile2()
{
    if (!canLaunchNew()) {
        return;
    }

    curProc_ = new QProcess(this);
    curType_ = COMPILE2;
    connect(curProc_, SIGNAL(finished(int)), this, SLOT(finished(int)));

    QStringList args;
    args.append("rl");
    args.append(tldict_);
    args.append(tlbin_);

    curProc_->start(appConfig().ltCompPath(), args);
}

void ToolsManager::compile3()
{
    if (!canLaunchNew()) {
        return;
    }

    curProc_ = new QProcess(this);
    curType_ = COMPILE3;
    connect(curProc_, SIGNAL(finished(int)), this, SLOT(finished(int)));

    QStringList args;
    args.append("lr");
    args.append(bidict_);
    args.append(bibin_);

    curProc_->start(appConfig().ltCompPath(), args);
}

void ToolsManager::compile4()
{
    if (!canLaunchNew()) {
        return;
    }

    curProc_ = new QProcess(this);
    curType_ = COMPILE4;
    connect(curProc_, SIGNAL(finished(int)), this, SLOT(finished(int)));

    QStringList args;
    args.append(rules_);
    args.append(rulebin_);

    curProc_->start(appConfig().apertiumPreprocTransPath(), args);
}

void ToolsManager::outputReady()
{
    curProc_->close();
}

void ToolsManager::finished(int retval)
{
    QString out = curProc_->readAllStandardOutput();
    curProc_->deleteLater();
    curProc_ = NULL;

    switch (curType_) {
    case COMPILE1:
        if (retval != 0) {
            emit compileFinished(false);
        } else {
            compile2();
        }
        break;
    case COMPILE2:
        if (retval != 0) {
            emit compileFinished(false);
        } else {
            compile3();
        }
        break;
    case COMPILE3:
        if (retval != 0) {
            emit compileFinished(false);
        } else {
            compile4();
        }
        break;
    case COMPILE4:
        emit compileFinished(retval == 0);
        break;

    case MORPH:
        {
            int lustart = out.indexOf("^");
            while (lustart != -1) {
                int sep = out.indexOf("/", lustart);
                out.remove(lustart+1, sep-lustart);
                lustart = out.indexOf("^", lustart+1);
            }
            emit morphFinished(out);
        }
        break;

    case GENERATE:
        emit generateFinished(out);
        break;

    case TRANSFER:
        emit transferFinished(out);
        break;

    default:
        break;
    }
}

QString ToolsManager::dirOfFile(const QString &str) const
{
    int lastslash = str.lastIndexOf("/");
    return str.mid(0, lastslash);
}
