/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <QTextStream>
#include <QString>
#include <QFile>

namespace Evernus
{
    class ChainableFileLogger final
    {
    public:
        ChainableFileLogger(const ChainableFileLogger &) = delete;
        ChainableFileLogger(ChainableFileLogger &&) = delete;

        ChainableFileLogger &operator =(const ChainableFileLogger &) = delete;
        ChainableFileLogger &operator =(ChainableFileLogger &&) = delete;

        static void initialize();

    private:
        static ChainableFileLogger *instance;

        QFile mLogFile;
        QTextStream mStream{&mLogFile};
        QtMessageHandler mPrevHandler = nullptr;

        explicit ChainableFileLogger(const QString &fileName);
        ~ChainableFileLogger();

        void writeMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);

        static void handleMessage(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    };
}
