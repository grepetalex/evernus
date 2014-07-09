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

#include <QDateTime>
#include <QString>

#include "Character.h"
#include "Entity.h"

namespace Evernus
{
    class CacheTimer
        : public Entity<uint>
    {
    public:
        using Entity::Entity;

        CacheTimer() = default;
        CacheTimer(IdType id, const QDateTime &dt);
        virtual ~CacheTimer() = default;

        Character::IdType getCharacterId() const noexcept;
        void setCharacterId(Character::IdType id) noexcept;

        QDateTime getCacheUntil() const;
        void setCacheUntil(const QDateTime &dt);

    private:
        Character::IdType mCharacterId = Character::invalidId;
        QDateTime mCacheUntil;
    };
}
