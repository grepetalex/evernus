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

#include <QWidget>
#include <QUrl>

class QWebEngineView;
class QWebEnginePage;
class QStackedWidget;
class QLineEdit;

namespace Evernus
{
    class SSOAuthWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        explicit SSOAuthWidget(const QUrl &url, QWidget *parent = nullptr);

        QWebEnginePage *page() const;

    signals:
        void acquiredCode(const QByteArray &code);

    private slots:
        void applyCode();

    private:
        QUrl mAuthUrl;

        QLineEdit *mUrlEdit = nullptr;
        QStackedWidget *mAuthWidgetStack = nullptr;
        QWebEngineView *mView = nullptr;
        QLineEdit *mCodeEdit = nullptr;
    };
}
