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
#include <unordered_set>

#include <QStandardItemModel>
#include <QDoubleValidator>
#include <QStackedWidget>
#include <QIntValidator>
#include <QApplication>
#include <QProgressBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QMessageBox>
#include <QTableView>
#include <QClipboard>
#include <QTabWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QSettings>
#include <QAction>
#include <QLabel>
#include <QDebug>

#include "TypeAggregatedDetailsWidget.h"
#include "MarketAnalysisSettings.h"
#include "StationSelectDialog.h"
#include "AdjustableTableView.h"
#include "EveDataProvider.h"
#include "ImportSettings.h"
#include "PriceSettings.h"
#include "SSOMessageBox.h"
#include "TaskManager.h"
#include "FlowLayout.h"
#include "UISettings.h"

#include "RegionAnalysisWidget.h"

namespace Evernus
{
    RegionAnalysisWidget::RegionAnalysisWidget(QByteArray clientId,
                                               QByteArray clientSecret,
                                               const EveDataProvider &dataProvider,
                                               const MarketDataProvider &marketDataProvider,
                                               QWidget *parent)
        : QWidget(parent)
        , mDataProvider(dataProvider)
        , mMarketDataProvider(marketDataProvider)
        , mTypeDataModel(mDataProvider)
        , mTypeViewProxy(TypeAggregatedMarketDataModel::getVolumeColumn(),
                         TypeAggregatedMarketDataModel::getMarginColumn(),
                         TypeAggregatedMarketDataModel::getBuyPriceColumn(),
                         TypeAggregatedMarketDataModel::getSellPriceColumn())
    {
        auto mainLayout = new QVBoxLayout{this};

        auto toolBarLayout = new FlowLayout{};
        mainLayout->addLayout(toolBarLayout);

        toolBarLayout->addWidget(new QLabel{tr("Region:"), this});

        mRegionCombo = new QComboBox{this};
        toolBarLayout->addWidget(mRegionCombo);
        mRegionCombo->setEditable(true);
        mRegionCombo->setInsertPolicy(QComboBox::NoInsert);

        QSettings settings;
        const auto lastRegion = settings.value(MarketAnalysisSettings::lastRegionKey).toUInt();

        const auto regions = mDataProvider.getRegions();
        for (const auto &region : regions)
        {
            mRegionCombo->addItem(region.second, region.first);
            if (region.first == lastRegion)
                mRegionCombo->setCurrentIndex(mRegionCombo->count() - 1);
        }

        connect(mRegionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &RegionAnalysisWidget::showForCurrentRegion);

        toolBarLayout->addWidget(new QLabel{tr("Limit to solar system:"), this});

        mSolarSystemCombo = new QComboBox{this};
        toolBarLayout->addWidget(mSolarSystemCombo);
        mSolarSystemCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
        mSolarSystemCombo->setEditable(true);
        mSolarSystemCombo->setInsertPolicy(QComboBox::NoInsert);
        fillSolarSystems(mRegionCombo->currentData().toUInt());
        connect(mSolarSystemCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &RegionAnalysisWidget::showForCurrentRegionAndSolarSystem);

        auto volumeValidator = new QIntValidator{this};
        volumeValidator->setBottom(0);

        toolBarLayout->addWidget(new QLabel{tr("Volume:"), this});

        auto value = settings.value(MarketAnalysisSettings::minVolumeFilterKey);

        mMinRegionVolumeEdit = new QLineEdit{(value.isValid()) ? (value.toString()) : (QString{}), this};
        toolBarLayout->addWidget(mMinRegionVolumeEdit);
        mMinRegionVolumeEdit->setValidator(volumeValidator);

        toolBarLayout->addWidget(new QLabel{"-", this});

        value = settings.value(MarketAnalysisSettings::maxVolumeFilterKey);

        mMaxRegionVolumeEdit = new QLineEdit{(value.isValid()) ? (value.toString()) : (QString{}), this};
        toolBarLayout->addWidget(mMaxRegionVolumeEdit);
        mMaxRegionVolumeEdit->setValidator(volumeValidator);

        auto marginValidator = new QIntValidator{this};

        toolBarLayout->addWidget(new QLabel{tr("Margin:"), this});

        value = settings.value(MarketAnalysisSettings::minMarginFilterKey);

        mMinRegionMarginEdit = new QLineEdit{(value.isValid()) ? (value.toString()) : (QString{}), this};
        toolBarLayout->addWidget(mMinRegionMarginEdit);
        mMinRegionMarginEdit->setValidator(marginValidator);
        mMinRegionMarginEdit->setPlaceholderText(locale().percent());

        toolBarLayout->addWidget(new QLabel{"-", this});

        value = settings.value(MarketAnalysisSettings::maxMarginFilterKey);

        mMaxRegionMarginEdit = new QLineEdit{(value.isValid()) ? (value.toString()) : (QString{}), this};
        toolBarLayout->addWidget(mMaxRegionMarginEdit);
        mMaxRegionMarginEdit->setValidator(marginValidator);
        mMaxRegionMarginEdit->setPlaceholderText(locale().percent());

        auto priceValidator = new QDoubleValidator{this};
        priceValidator->setBottom(0.);

        toolBarLayout->addWidget(new QLabel{tr("Buy price:"), this});

        value = settings.value(MarketAnalysisSettings::minBuyPriceFilterKey);

        mMinBuyPriceEdit = new QLineEdit{(value.isValid()) ? (value.toString()) : (QString{}), this};
        toolBarLayout->addWidget(mMinBuyPriceEdit);
        mMinBuyPriceEdit->setValidator(priceValidator);

        toolBarLayout->addWidget(new QLabel{"-", this});

        value = settings.value(MarketAnalysisSettings::maxBuyPriceFilterKey);

        mMaxBuyPriceEdit = new QLineEdit{(value.isValid()) ? (value.toString()) : (QString{}), this};
        toolBarLayout->addWidget(mMaxBuyPriceEdit);
        mMaxBuyPriceEdit->setValidator(priceValidator);

        toolBarLayout->addWidget(new QLabel{tr("Sell price:"), this});

        value = settings.value(MarketAnalysisSettings::minSellPriceFilterKey);

        mMinSellPriceEdit = new QLineEdit{(value.isValid()) ? (value.toString()) : (QString{}), this};
        toolBarLayout->addWidget(mMinSellPriceEdit);
        mMinSellPriceEdit->setValidator(priceValidator);

        toolBarLayout->addWidget(new QLabel{"-", this});

        value = settings.value(MarketAnalysisSettings::maxSellPriceFilterKey);

        mMaxSellPriceEdit = new QLineEdit{(value.isValid()) ? (value.toString()) : (QString{}), this};
        toolBarLayout->addWidget(mMaxSellPriceEdit);
        mMaxSellPriceEdit->setValidator(priceValidator);

        auto filterBtn = new QPushButton{tr("Apply"), this};
        toolBarLayout->addWidget(filterBtn);
        connect(filterBtn, &QPushButton::clicked, this, &RegionAnalysisWidget::applyRegionFilter);

        mainLayout->addWidget(new QLabel{tr("Double-click an item for additional information. \"Show in EVE\" is available via the right-click menu. Remember to select desired <b>source price</b> and <b>destination price</b> from the dropdowns at the top, otherwise your differences might be skewed."), this});

        mRegionDataStack = new QStackedWidget{this};
        mainLayout->addWidget(mRegionDataStack);

        auto waitingLabel = new QLabel{tr("Calculating data..."), this};
        mRegionDataStack->addWidget(waitingLabel);
        waitingLabel->setAlignment(Qt::AlignCenter);

        mTypeViewProxy.setSortRole(Qt::UserRole);
        mTypeViewProxy.setSourceModel(&mTypeDataModel);

        mRegionTypeDataView = new AdjustableTableView{"marketAnalysisRegionView", this};
        mRegionDataStack->addWidget(mRegionTypeDataView);
        mRegionTypeDataView->setSortingEnabled(true);
        mRegionTypeDataView->setAlternatingRowColors(true);
        mRegionTypeDataView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        mRegionTypeDataView->setModel(&mTypeViewProxy);
        mRegionTypeDataView->setContextMenuPolicy(Qt::ActionsContextMenu);
        mRegionTypeDataView->restoreHeaderState();
        connect(mRegionTypeDataView, &QTableView::doubleClicked, this, &RegionAnalysisWidget::showDetails);
        connect(mRegionTypeDataView->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &RegionAnalysisWidget::selectRegionType);

        mRegionDataStack->setCurrentWidget(mRegionTypeDataView);

        mShowDetailsAct = new QAction{tr("Show details"), this};
        mShowDetailsAct->setEnabled(false);
        mRegionTypeDataView->addAction(mShowDetailsAct);
        connect(mShowDetailsAct, &QAction::triggered, this, &RegionAnalysisWidget::showDetailsForCurrent);

        mShowInEveRegionAct = new QAction{tr("Show in EVE"), this};
        mShowInEveRegionAct->setEnabled(false);
        mRegionTypeDataView->addAction(mShowInEveRegionAct);
        connect(mShowInEveRegionAct, &QAction::triggered, this, &RegionAnalysisWidget::showInEveForCurrentRegion);

        mCopyRegionRowsAct = new QAction{tr("&Copy"), this};
        mCopyRegionRowsAct->setEnabled(false);
        mCopyRegionRowsAct->setShortcut(QKeySequence::Copy);
        connect(mCopyRegionRowsAct, &QAction::triggered, this, [=] {
            copyRows(*mRegionTypeDataView, mTypeViewProxy);
        });
        mRegionTypeDataView->addAction(mCopyRegionRowsAct);
    }

    void RegionAnalysisWidget::setPriceTypes(PriceType src, PriceType dst) noexcept
    {
        mSrcPriceType = src;
        mDstPriceType = dst;
    }

    void RegionAnalysisWidget::setBogusOrderThreshold(double value) noexcept
    {
        mTypeDataModel.setBogusOrderThreshold(value);
    }

    void RegionAnalysisWidget::discardBogusOrders(bool flag) noexcept
    {
        mTypeDataModel.discardBogusOrders(flag);
    }

    void RegionAnalysisWidget::setCharacter(const std::shared_ptr<Character> &character)
    {
        mTypeDataModel.setCharacter(character);
    }

    void RegionAnalysisWidget::showForCurrentRegion()
    {
        const auto region = getCurrentRegion();
        if (region != 0)
        {
            QSettings settings;
            settings.setValue(MarketAnalysisSettings::lastRegionKey, region);

            mRegionDataStack->setCurrentIndex(waitingLabelIndex);
            mRegionDataStack->repaint();

            const auto historyAndOrders = getHistoryAndOrders(region);

            fillSolarSystems(region);
            mTypeDataModel.setOrderData(*historyAndOrders.second,
                                        *historyAndOrders.first,
                                        region,
                                        mSrcPriceType,
                                        mDstPriceType);

            mRegionDataStack->setCurrentWidget(mRegionTypeDataView);
        }
    }

    void RegionAnalysisWidget::showForCurrentRegionAndSolarSystem()
    {
        const auto region = getCurrentRegion();
        if (region != 0)
        {
            mRegionDataStack->setCurrentIndex(waitingLabelIndex);
            mRegionDataStack->repaint();

            const auto historyAndOrders = getHistoryAndOrders(region);

            const auto system = mSolarSystemCombo->currentData().toUInt();
            mTypeDataModel.setOrderData(*historyAndOrders.second,
                                        *historyAndOrders.first,
                                        region,
                                        mSrcPriceType,
                                        mDstPriceType,
                                        system);

            mRegionDataStack->setCurrentWidget(mRegionTypeDataView);
        }
    }

    void RegionAnalysisWidget::applyRegionFilter()
    {
        const auto minVolume = mMinRegionVolumeEdit->text();
        const auto maxVolume = mMaxRegionVolumeEdit->text();
        const auto minMargin = mMinRegionMarginEdit->text();
        const auto maxMargin = mMaxRegionMarginEdit->text();
        const auto minBuyPrice = mMinBuyPriceEdit->text();
        const auto maxBuyPrice = mMaxBuyPriceEdit->text();
        const auto minSellPrice = mMinSellPriceEdit->text();
        const auto maxSellPrice = mMaxSellPriceEdit->text();

        QSettings settings;
        settings.setValue(MarketAnalysisSettings::minVolumeFilterKey, minVolume);
        settings.setValue(MarketAnalysisSettings::maxVolumeFilterKey, maxVolume);
        settings.setValue(MarketAnalysisSettings::minMarginFilterKey, minMargin);
        settings.setValue(MarketAnalysisSettings::maxMarginFilterKey, maxMargin);
        settings.setValue(MarketAnalysisSettings::minBuyPriceFilterKey, minBuyPrice);
        settings.setValue(MarketAnalysisSettings::maxBuyPriceFilterKey, maxBuyPrice);
        settings.setValue(MarketAnalysisSettings::minSellPriceFilterKey, minSellPrice);
        settings.setValue(MarketAnalysisSettings::maxSellPriceFilterKey, maxSellPrice);

        mTypeViewProxy.setFilter((minVolume.isEmpty()) ? (TypeAggregatedMarketDataFilterProxyModel::VolumeValueType{}) : (minVolume.toUInt()),
                                 (maxVolume.isEmpty()) ? (TypeAggregatedMarketDataFilterProxyModel::VolumeValueType{}) : (maxVolume.toUInt()),
                                 (minMargin.isEmpty()) ? (TypeAggregatedMarketDataFilterProxyModel::MarginValueType{}) : (minMargin.toDouble()),
                                 (maxMargin.isEmpty()) ? (TypeAggregatedMarketDataFilterProxyModel::MarginValueType{}) : (maxMargin.toDouble()),
                                 (minBuyPrice.isEmpty()) ? (TypeAggregatedMarketDataFilterProxyModel::PriceValueType{}) : (minBuyPrice.toDouble()),
                                 (maxBuyPrice.isEmpty()) ? (TypeAggregatedMarketDataFilterProxyModel::PriceValueType{}) : (maxBuyPrice.toDouble()),
                                 (minSellPrice.isEmpty()) ? (TypeAggregatedMarketDataFilterProxyModel::PriceValueType{}) : (minSellPrice.toDouble()),
                                 (maxSellPrice.isEmpty()) ? (TypeAggregatedMarketDataFilterProxyModel::PriceValueType{}) : (maxSellPrice.toDouble()));
    }

    void RegionAnalysisWidget::showDetails(const QModelIndex &item)
    {
        const auto id = mTypeDataModel.getTypeId(mTypeViewProxy.mapToSource(item));
        const auto region = getCurrentRegion();
        const auto history = mMarketDataProvider.getHistory(region);
        if (history == nullptr)
            return;

        const auto it = history->find(id);
        if (it != std::end(*history))
        {
            auto widget = new TypeAggregatedDetailsWidget{it->second, this, Qt::Window};
            widget->setWindowTitle(tr("%1 in %2").arg(mDataProvider.getTypeName(id)).arg(mDataProvider.getRegionName(region)));
            widget->show();
            connect(this, &RegionAnalysisWidget::preferencesChanged, widget, &TypeAggregatedDetailsWidget::handleNewPreferences);
        }
    }

    void RegionAnalysisWidget::selectRegionType(const QItemSelection &selected)
    {
        const auto enabled = !selected.isEmpty();
        mShowDetailsAct->setEnabled(enabled);
        mShowInEveRegionAct->setEnabled(enabled);
        mCopyRegionRowsAct->setEnabled(enabled);
    }

    void RegionAnalysisWidget::showDetailsForCurrent()
    {
        showDetails(mRegionTypeDataView->currentIndex());
    }

    void RegionAnalysisWidget::showInEveForCurrentRegion()
    {
        const auto index = mTypeViewProxy.mapToSource(mRegionTypeDataView->currentIndex());
        const auto id = mTypeDataModel.getTypeId(index);
        if (id != EveType::invalidId)
            emit showInEve(id, mTypeDataModel.getOwnerId(index));
    }

    void RegionAnalysisWidget::copyRows(const QAbstractItemView &view, const QAbstractItemModel &model) const
    {
        const auto indexes = view.selectionModel()->selectedIndexes();
        if (indexes.isEmpty())
            return;

        QSettings settings;
        const auto delim
            = settings.value(UISettings::columnDelimiterKey, UISettings::columnDelimiterDefault).value<char>();

        QString result;

        auto prevRow = indexes.first().row();
        for (const auto &index : indexes)
        {
            if (prevRow != index.row())
            {
                prevRow = index.row();
                result[result.size() - 1] = '\n';
            }

            result.append(model.data(index).toString());
            result.append(delim);
        }

        result.chop(1);
        QApplication::clipboard()->setText(result);
    }

    void RegionAnalysisWidget::fillSolarSystems(uint regionId)
    {
        mSolarSystemCombo->blockSignals(true);
        mSolarSystemCombo->clear();

        mSolarSystemCombo->addItem(tr("- all -"));

        const auto &systems = mDataProvider.getSolarSystemsForRegion(regionId);
        for (const auto &system : systems)
            mSolarSystemCombo->addItem(system.second, system.first);

        mSolarSystemCombo->setCurrentIndex(0);
        mSolarSystemCombo->blockSignals(false);
    }

    uint RegionAnalysisWidget::getCurrentRegion() const
    {
        return mRegionCombo->currentData().toUInt();
    }

    RegionAnalysisWidget::HistoryOrdersPair RegionAnalysisWidget::getHistoryAndOrders(uint region)
    {
        auto history = mMarketDataProvider.getHistory(region);
        if (history == nullptr)
            history = &mEmptyHistory;

        auto orders = mMarketDataProvider.getOrders();
        if (orders == nullptr)
            orders = &mEmptyOrders;

        return std::make_pair(history, orders);
    }
}
