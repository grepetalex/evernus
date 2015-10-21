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

#include "ScriptOrderProcessingModel.h"
#include "AggregatedStatisticsModel.h"

class QRadioButton;
class QPushButton;
class QTableView;
class QTextEdit;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QCPBars;
class QLabel;

namespace Evernus
{
    class CorpMarketOrderValueSnapshotRepository;
    class MarketOrderValueSnapshotRepository;
    class WalletJournalEntryRepository;
    class AssetValueSnapshotRepository;
    class CorpWalletSnapshotRepository;
    class WalletTransactionRepository;
    class WalletSnapshotRepository;
    class DateFilteredPlotWidget;
    class OrderScriptRepository;
    class CharacterRepository;

    class StatisticsWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        StatisticsWidget(const AssetValueSnapshotRepository &assetSnapshotRepo,
                         const WalletSnapshotRepository &walletSnapshotRepo,
                         const CorpWalletSnapshotRepository &corpWalletSnapshotRepo,
                         const MarketOrderValueSnapshotRepository &marketOrderSnapshotRepo,
                         const CorpMarketOrderValueSnapshotRepository &corpMarketOrderSnapshotRepo,
                         const WalletJournalEntryRepository &journalRepo,
                         const WalletTransactionRepository &transactionRepo,
                         const WalletJournalEntryRepository &corpJournalRepo,
                         const WalletTransactionRepository &corpTransactionRepo,
                         const MarketOrderRepository &orderRepo,
                         const OrderScriptRepository &orderScriptRepo,
                         const CharacterRepository &characterRepo,
                         const EveDataProvider &dataProvider,
                         QWidget *parent = nullptr);
        virtual ~StatisticsWidget() = default;

    signals:
        void makeSnapshots();

    public slots:
        void setCharacter(Character::IdType id);
        void updateBalanceData();
        void updateJournalData();
        void updateTransactionData();
        void updateData();
        void handleNewPreferences();

    private slots:
        void applyAggrFilter();
        void applyScript();

        void copyAggrData();

        void showScriptError(const QString &message);

        void saveScript();
        void loadScript();
        void deleteScript();

        void updateBalanceTooltip(QMouseEvent *event);

    private:
        enum
        {
            assetValueGraph,
            walletBalanceGraph,
            corpWalletBalanceGraph,
            buyOrdersGraph,
            sellOrdersGraph,
            totalValueGraph,
        };

        const AssetValueSnapshotRepository &mAssetSnapshotRepository;
        const WalletSnapshotRepository &mWalletSnapshotRepository;
        const CorpWalletSnapshotRepository &mCorpWalletSnapshotRepository;
        const MarketOrderValueSnapshotRepository &mMarketOrderSnapshotRepository;
        const CorpMarketOrderValueSnapshotRepository &mCorpMarketOrderSnapshotRepository;
        const WalletJournalEntryRepository &mJournalRepository, &mCorpJournalRepository;
        const WalletTransactionRepository &mTransactionRepository, &mCorpTransactionRepository;
        const MarketOrderRepository &mMarketOrderRepository;
        const OrderScriptRepository &mOrderScriptRepository;
        const CharacterRepository &mCharacterRepository;

        DateFilteredPlotWidget *mBalancePlot = nullptr;
        DateFilteredPlotWidget *mJournalPlot = nullptr;
        DateFilteredPlotWidget *mTransactionPlot = nullptr;

        QCPBars *mIncomingPlot = nullptr;
        QCPBars *mOutgoingPlot = nullptr;

        QCPBars *mBuyPlot = nullptr;
        QCPBars *mSellPlot = nullptr;

        QCheckBox *mCombineStatsBtn = nullptr;
        QPushButton *mAggrApplyBtn = nullptr;
        QPushButton *mScriptApplyBtn = nullptr;
        QComboBox *mAggrGroupingColumnCombo = nullptr;
        QComboBox *mAggrOrderColumnCombo = nullptr;
        QSpinBox *mAggrLimitEdit = nullptr;
        QCheckBox *mAggrIncludeActiveBtn = nullptr;
        QCheckBox *mAggrIncludeNotFulfilledBtn = nullptr;
        QTextEdit *mAggrScriptEdit = nullptr;
        QRadioButton *mScriptForEachModeBtn = nullptr;
        QTableView *mAggrView = nullptr;
        QLabel *mJournalIncomeLabel = nullptr;
        QLabel *mJournalOutcomeLabel = nullptr;
        QLabel *mJournalBalanceLabel = nullptr;
        QLabel *mTransactionsIncomeLabel = nullptr;
        QLabel *mTransactionsOutcomeLabel = nullptr;
        QLabel *mTransactionsBalanceLabel = nullptr;

        Character::IdType mCharacterId = Character::invalidId;

        AggregatedStatisticsModel mAggrModel;
        ScriptOrderProcessingModel mScriptModel;

        QString mLastLoadedScript;

        void updateGraphAndLegend();
        void updateGraphColors();

        QWidget *createBasicStatisticsWidget();
        QWidget *createAdvancedStatisticsWidget();

        DateFilteredPlotWidget *createPlot();

        static QCPBars *createBarPlot(DateFilteredPlotWidget *plot, const QString &name, Qt::GlobalColor color);
        static void createBarTicks(QVector<double> &ticks,
                                   QVector<double> &incomingTicks,
                                   QVector<double> &outgoingTicks,
                                   QVector<double> &incomingValues,
                                   QVector<double> &outgoingValues,
                                   const QHash<QDate, std::pair<double, double>> &values);
    };
}
