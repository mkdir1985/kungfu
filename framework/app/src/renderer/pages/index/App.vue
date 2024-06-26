<script setup lang="ts">
import { getCurrentInstance, onBeforeUnmount, onMounted, ref } from 'vue';
import KfSystemPrepareModal from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfSystemPrepareModal.vue';
import KfLayoutVue from '@kungfu-trader/kungfu-app/src/renderer/components/layout/KfLayout.vue';
import KfSetByConfigModal from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfSetByConfigModal.vue';
import { Locale } from 'ant-design-vue/es/locale-provider';
import zhCN from 'ant-design-vue/es/locale/zh_CN';
import { langDefault } from '@kungfu-trader/kungfu-js-api/language';
import {
  markClearJournal,
  removeLoadingMask,
  useIpcListener,
  handleOpenLogviewByFile,
  markClearDB,
  setHtmlTitle,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import {
  playSound,
  useDealExportHistoryTradingData,
  useDealInstruments,
  usePreStartAndQuitApp,
  useSubscibeInstrumentAtEntry,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';

import { useGlobalStore } from './store/global';
import KfDownloadDateModal from '@kungfu-trader/kungfu-app/src/renderer/components/layout/KfHistoryDateModal.vue';
import { tradingDataSubject } from '@kungfu-trader/kungfu-js-api/kungfu/tradingData';
import globalBus from '@kungfu-trader/kungfu-js-api/utils/globalBus';
import {
  dealAppStates,
  dealAssetsByHolderUID,
  dealStrategyStates,
  getTradingDataSortKey,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { bindIPCListener } from '@kungfu-trader/kungfu-app/src/renderer/ipcMsg/ipcListener';
import { useTradingTask } from '@kungfu-trader/kungfu-app/src/components/modules/tradingTask/utils';
import { setAllRiskSettingList } from '@kungfu-trader/kungfu-js-api/actions';

setHtmlTitle();

const app = getCurrentInstance();
const store = useGlobalStore();
const locale = ref<Locale>();

const {
  preStartSystemLoadingData,
  preStartSystemLoading,
  preQuitSystemLoadingData,
  preQuitSystemLoading,
} = usePreStartAndQuitApp();

useDealInstruments();
useSubscibeInstrumentAtEntry(window.watcher);

const { exportDateModalVisible, exportDataLoading, handleConfirmExportDate } =
  useDealExportHistoryTradingData();

const latestTrade = ref<bigint>(0n);

useIpcListener();

const tradingDataSubscription = tradingDataSubject.subscribe(
  (watcher: KungfuApi.Watcher) => {
    const appStates = dealAppStates(watcher, watcher.appStates);
    store.setAppStates(appStates);
    const assets = dealAssetsByHolderUID<KungfuApi.Asset>(
      watcher,
      watcher.ledger.Asset,
    );
    store.setAssets(assets);
    const strategyStates = dealStrategyStates(watcher, watcher.strategyStates);
    store.setStrategyStates(strategyStates);

    const sortKey = getTradingDataSortKey('Trade');
    const trades = watcher.ledger.Trade.sort(sortKey);
    if (trades.length && latestTrade.value !== trades[0]?.trade_id) {
      latestTrade.value = trades[0]?.trade_id || 0n;
      playSound();
    }
  },
);

store.setKfConfigList();
store.setKfExtConfigs();
store.setSubscribedInstrumentsByLocal();
store.setRiskSettingList();
store.setKfGlobalSetting();

const busSubscription = globalBus.subscribe((data: KfEvent.KfBusEvent) => {
  if (data.tag === 'main') {
    switch (data.name) {
      case 'clear-journal':
        markClearJournal();
        break;
      case 'clear-db':
        markClearDB();
        break;
      case 'open-log':
        handleOpenLogviewByFile();
        break;
      case 'export-all-trading-data':
        globalBus.next({
          tag: 'export',
          tradingDataType: 'all',
        } as KfEvent.ExportTradingDataEvent);
    }
  }
  if (data.tag === 'update:riskSetting') {
    setAllRiskSettingList(data.riskSettings).finally(() => {
      store.setRiskSettingList();
    });
  }
  if (data.tag === 'play:tradingError') {
    playSound();
  }
  if (data.tag === 'orderbook') {
    store.setOrderBookCurrentInstrument(data.instrument);
  }
});

const {
  setTradingTaskModalVisible,
  currentSelectedTradingTaskExtKey,
  setTradingTaskConfigPayload,
  handleConfirmAddUpdateTask,
} = useTradingTask();

onMounted(() => {
  locale.value =
    (app?.proxy?.$antLocalesMap || {})[
      store.globalSetting?.system?.language || langDefault
    ] || zhCN;

  bindIPCListener(store);
  removeLoadingMask();

  window.addEventListener('resize', () => {
    app?.proxy &&
      app?.proxy.$globalBus.next({
        tag: 'resize',
      } as KfEvent.ResizeEvent);
  });

  app?.proxy?.$globalBus.next({ tag: 'appMounted' });
});

onBeforeUnmount(() => {
  tradingDataSubscription.unsubscribe();
  busSubscription.unsubscribe();
});
</script>

<template>
  <a-config-provider :locale="locale" :autoInsertSpaceInButton="false">
    <div class="app__warp">
      <KfLayoutVue>
        <router-view />
      </KfLayoutVue>
    </div>

    <KfSystemPrepareModal
      :title="$t('system_prompt')"
      :visible="preStartSystemLoading"
      :status="[
        {
          key: 'cpusSafeNumChecking',
          status: preStartSystemLoadingData.cpusSafeNumChecking,
        },
        { key: 'archive', status: preStartSystemLoadingData.archive },
        { key: 'watcher', status: preStartSystemLoadingData.watcher },
        {
          key: 'extraResourcesLoading',
          status: preStartSystemLoadingData.extraResourcesLoading,
        },
      ]"
      :txt="{
        cpusSafeNumChecking: {
          done: $t('computer_performance_done'),
          loading: $t('computer_performance_detecting'),
        },
        archive: { done: $t('archive_done'), loading: $t('archive_loading') },
        watcher: {
          done: $t('environment_done'),
          loading: $t('environment_loading'),
        },
        extraResourcesLoading: {
          done: $t('extra_resources_done'),
          loading: $t('extra_resouces_loading'),
        },
      }"
    ></KfSystemPrepareModal>
    <KfSystemPrepareModal
      :title="$t('system_prompt')"
      :visible="preQuitSystemLoading"
      :status="[
        {
          key: 'record',
          status: preQuitSystemLoadingData.record,
        },
        {
          key: 'quit',
          status: preQuitSystemLoadingData.quit,
        },
      ]"
      :txt="{
        record: {
          done: $t('saving_data_done'),
          loading: $t('saving_data_loading'),
        },
        quit: {
          done: $t('end_all_transactions'),
          loading: $t('closing'),
        },
      }"
    ></KfSystemPrepareModal>

    <!-- global modal start -->

    <!-- export trading data -->
    <KfDownloadDateModal
      v-if="exportDateModalVisible"
      v-model:visible="exportDateModalVisible"
      @confirm="handleConfirmExportDate"
    ></KfDownloadDateModal>

    <!-- add/update trading task -->
    <KfSetByConfigModal
      v-if="setTradingTaskModalVisible"
      v-model:visible="setTradingTaskModalVisible"
      :payload="setTradingTaskConfigPayload"
      :isPrimaryDisabled="true"
      :passPrimaryKeySpecialWordsVerify="true"
      @confirm="
        handleConfirmAddUpdateTask($event, currentSelectedTradingTaskExtKey)
      "
    ></KfSetByConfigModal>

    <!-- global modal end -->

    <a-spin v-if="exportDataLoading" :spinning="exportDataLoading"></a-spin>
  </a-config-provider>
</template>

<style lang="less">
@import '@kungfu-trader/kungfu-app/src/renderer/assets/less/coverAnt.less';
@import '@kungfu-trader/kungfu-app/src/renderer/assets/less/base.less';
@import '@kungfu-trader/kungfu-app/src/renderer/assets/less/public.less';

#app {
  width: 100%;
  height: 100%;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;

  .app__warp {
    height: 100%;
    width: 100%;
  }
}
</style>
