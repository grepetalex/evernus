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

#include <unordered_map>
#include <functional>
#include <vector>
#include <memory>
#include <chrono>

#include <QAbstractItemModel>

#include "IndustryManufacturingSetup.h"
#include "CharacterRepository.h"
#include "EveDataProvider.h"
#include "IndustryUtils.h"
#include "Character.h"
#include "EveType.h"

namespace Evernus
{
    class AssetProvider;

    class IndustryManufacturingSetupModel
        : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        IndustryManufacturingSetupModel(IndustryManufacturingSetup &setup,
                                        const EveDataProvider &dataProvider,
                                        const AssetProvider &assetProvider,
                                        const CharacterRepository &characterRepo,
                                        QObject *parent = nullptr);
        IndustryManufacturingSetupModel(const IndustryManufacturingSetupModel &) = default;
        IndustryManufacturingSetupModel(IndustryManufacturingSetupModel &&) = default;
        virtual ~IndustryManufacturingSetupModel() = default;

        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex{}) const override;
        virtual QModelIndex parent(const QModelIndex &index) const override;
        virtual QHash<int, QByteArray> roleNames() const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        void refreshData();
        void refreshAssets();

        void setSource(EveType::IdType id, IndustryManufacturingSetup::InventorySource source);
        void setRuns(EveType::IdType id, uint runs);
        void setMaterialEfficiency(EveType::IdType id, uint value);
        void setTimeEfficiency(EveType::IdType id, uint value);

        void setCharacter(Character::IdType id);
        void setFacilityType(IndustryUtils::FacilityType type);
        void setSecurityStatus(IndustryUtils::SecurityStatus status);
        void setMaterialRigType(IndustryUtils::RigType type);
        void setTimeRigType(IndustryUtils::RigType type);
        void setFacilitySize(IndustryUtils::Size size);

        IndustryManufacturingSetupModel &operator =(const IndustryManufacturingSetupModel &) = default;
        IndustryManufacturingSetupModel &operator =(IndustryManufacturingSetupModel &&) = default;

    private:
        enum
        {
            NameRole = Qt::UserRole,
            TypeIdRole,
            QuantityProducedRole,
            QuantityRequiredRole,
            SourceRole,
            TimeRole,
            RunsRole,
            MaterialEfficiencyRole,
            TimeEfficiencyRole,
        };

        class TreeItem;

        using TreeItemPtr = std::unique_ptr<TreeItem>;

        class TreeItem final
        {
        public:
            TreeItem(IndustryManufacturingSetupModel &model,
                     const IndustryManufacturingSetup &setup);
            TreeItem(EveType::IdType typeId,
                     IndustryManufacturingSetupModel &model,
                     const IndustryManufacturingSetup &setup);
            ~TreeItem() = default;

            EveType::IdType getTypeId() const noexcept;

            quint64 getEffectiveQuantityRequired() const;
            quint64 getQuantityRequiredForParent() const;

            uint getQuantityRequired() const noexcept;
            void setQuantityRequired(uint value) noexcept;

            uint getQuantityProduced() const noexcept;

            uint getEffectiveRuns() const;
            uint getRuns() const noexcept;
            void setRuns(uint value) noexcept;

            std::chrono::seconds getEffectiveTime() const;

            TreeItem *getChild(int row) const;
            TreeItem *getParent() const noexcept;

            int getRow() const noexcept;

            int getChildCount() const noexcept;

            void appendChild(std::unique_ptr<TreeItem> child);
            void clearChildren() noexcept;

            inline auto begin() noexcept
            {
                return std::begin(mChildItems);
            }

            inline auto begin() const noexcept
            {
                return std::begin(mChildItems);
            }

            inline auto end() noexcept
            {
                return std::end(mChildItems);
            }

            inline auto end() const noexcept
            {
                return std::end(mChildItems);
            }

        private:
            IndustryManufacturingSetupModel &mModel;
            const IndustryManufacturingSetup &mSetup;
            TreeItem *mParent = nullptr;
            EveType::IdType mTypeId = EveType::invalidId;
            uint mQuantityRequired = 0;
            uint mRuns = 1;
            Evernus::EveDataProvider::ManufacturingInfo mManufacturingInfo;
            std::vector<TreeItemPtr> mChildItems;

            uint getMaterialEfficiency() const;
            uint getTimeEfficiency() const;

            bool isOutput() const;

            std::chrono::seconds getTimeToManufacture() const;
        };

        struct AssetQuantity
        {
            quint64 mInitialQuantity = 0;
            quint64 mCurrentQuantity = 0;
        };

        IndustryManufacturingSetup &mSetup;
        const EveDataProvider &mDataProvider;
        const AssetProvider &mAssetProvider;
        const CharacterRepository &mCharacterRepo;

        TreeItem mRoot{*this, mSetup};

        CharacterRepository::EntityPtr mCharacter;
        std::unordered_map<uint, int> mCharacterManufacturingSkills;

        std::unordered_multimap<EveType::IdType, std::reference_wrapper<TreeItem>> mTypeItemMap;

        std::unordered_map<EveType::IdType, AssetQuantity> mAssetQuantities;

        IndustryUtils::FacilityType mFacilityType = IndustryUtils::FacilityType::Station;
        IndustryUtils::SecurityStatus mSecurityStatus = IndustryUtils::SecurityStatus::HighSec;
        IndustryUtils::RigType mMaterialRigType = IndustryUtils::RigType::None;
        IndustryUtils::RigType mTimeRigType = IndustryUtils::RigType::None;
        IndustryUtils::Size mFacilitySize = IndustryUtils::Size::Medium;


        void fillChildren(TreeItem &item);

        TreeItemPtr createOutputItem(EveType::IdType typeId,
                                     const IndustryManufacturingSetup::OutputSettings &settings);
        TreeItemPtr createSourceItem(const EveDataProvider::MaterialInfo &materialInfo);

        template<class Iterator>
        void fillAssetList(Iterator begin, Iterator end);

        quint64 takeAssets(EveType::IdType typeId, quint64 max);

        void signalQuantityChange(EveType::IdType typeId);
        void signalTimeChange(EveType::IdType typeId);
        void signalRoleChange(const QVector<int> &roles);
        void signalRoleChange(EveType::IdType typeId, const QVector<int> &roles);
        void signalQuantityChange();
        void signalTimeChange();
        void signalRoleChange(TreeItem &item, const QVector<int> &roles);
        void roleAndQuantityChange(EveType::IdType typeId, const QVector<int> &roles);
    };
}
