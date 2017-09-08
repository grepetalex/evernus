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
#include <QSettings>

#include "NetworkSettings.h"
#include "ESIInterface.h"

#include "ESIInterfaceManager.h"

namespace Evernus
{
    std::vector<std::unique_ptr<ESIInterface, QObjectDeleteLaterDeleter>> ESIInterfaceManager::mInterfaces;
    std::size_t ESIInterfaceManager::mCurrentInterface = 0;

    ESIInterfaceManager::ESIInterfaceManager(QObject *parent)
        : QObject{parent}
    {
        if (Q_UNLIKELY(mInterfaces.empty()))
            createInterfaces();

        connectInterfaces();
    }

    void ESIInterfaceManager::handleNewPreferences()
    {
        mInterfaces.clear();

        createInterfaces();
        connectInterfaces();
    }

    const ESIInterface &ESIInterfaceManager::selectNextInterface()
    {
        const auto &interface = *mInterfaces[mCurrentInterface];
        mCurrentInterface = (mCurrentInterface + 1) % mInterfaces.size();

        return interface;
    }

    void ESIInterfaceManager::connectInterfaces()
    {
        for (const auto &interface : mInterfaces)
        {
            connect(interface.get(), &ESIInterface::tokenRequested, this, &ESIInterfaceManager::tokenRequested, Qt::QueuedConnection);
            connect(this, &ESIInterfaceManager::acquiredToken, interface.get(), &ESIInterface::updateTokenAndContinue);
            connect(this, &ESIInterfaceManager::tokenError, interface.get(), &ESIInterface::handleTokenError);
        }
    }

    void ESIInterfaceManager::createInterfaces()
    {
        QSettings settings;

        // IO bound
        const auto maxInterfaces = std::max(
            settings.value(NetworkSettings::maxESIThreadsKey, NetworkSettings::maxESIThreadsDefault).toUInt(),
            1u
        );

        for (auto i = 0u; i < maxInterfaces; ++i)
            mInterfaces.emplace_back(new ESIInterface{});
    }
}
