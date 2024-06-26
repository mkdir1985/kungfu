<script setup lang="ts">
import {
  dealKfPrice,
  dealAssetPrice,
  dealDirection,
  dealCurrency,
  isTdStrategyCategory,
  getIdByKfLocation,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  useDownloadHistoryTradingData,
  useTableSearchKeyword,
  useDashboardBodySize,
  useTriggerMakeOrder,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfTradingDataTable from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfTradingDataTable.vue';
import { DownloadOutlined, ReloadOutlined } from '@ant-design/icons-vue';

import {
  computed,
  getCurrentInstance,
  onBeforeUnmount,
  onMounted,
  ref,
  toRaw,
  watch,
} from 'vue';
import { storeToRefs } from 'pinia';
import { getColumns } from './config';
import KfBlinkNum from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfBlinkNum.vue';
import { dealPosition } from '@kungfu-trader/kungfu-js-api/kungfu';
import { useGlobalStore } from '@kungfu-trader/kungfu-app/src/renderer/pages/index/store/global';
import { SideEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import {
  getInstrumentByInstrumentPair,
  useCurrentGlobalKfLocation,
  useInstruments,
  useDealDataWithCaches,
  useQuote,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { messagePrompt } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { getPosClosableVolumeByOffset, resolveTriggerOffset } from './utils';

const { t } = VueI18n.global;
const { success, error } = messagePrompt();
const app = getCurrentInstance();
const { handleBodySizeChange } = useDashboardBodySize();

const pos = ref<KungfuApi.PositionResolved[]>([]);
const { searchKeyword, tableData } =
  useTableSearchKeyword<KungfuApi.PositionResolved>(pos, [
    'instrument_id_resolved',
    'exchange_id',
    'direction',
    'account_id_resolved',
  ]);
const {
  currentGlobalKfLocation,
  currentCategoryData,
  getCurrentGlobalKfLocationId,
} = useCurrentGlobalKfLocation(window.watcher);
const { handleDownload } = useDownloadHistoryTradingData();
const { triggerOrderBook, triggerMakeOrder } = useTriggerMakeOrder();
const { instruments } = useInstruments();
const { getPositionLastPrice } = useQuote();
const { dealDataWithCache } = useDealDataWithCaches<
  KungfuApi.Position,
  KungfuApi.PositionResolved
>(['uid_key', 'update_time']);
const { globalSetting } = storeToRefs(useGlobalStore());

const columns = computed(() => {
  if (currentGlobalKfLocation.value === null) {
    return getColumns({
      category: 'td',
      group: '*',
      name: '*',
      mode: 'live',
    });
  }

  return getColumns(currentGlobalKfLocation.value);
});

onMounted(() => {
  if (app?.proxy) {
    const subscription = app.proxy.$tradingDataSubject.subscribe(
      (watcher: KungfuApi.Watcher) => {
        if (currentGlobalKfLocation.value === null) {
          return;
        }

        const positions =
          globalThis.HookKeeper.getHooks().dealTradingData.trigger(
            watcher,
            currentGlobalKfLocation.value,
            watcher.ledger.Position,
            'position',
          ) as KungfuApi.Position[];

        pos.value = toRaw(
          positions
            .reverse()
            .map((item) =>
              dealDataWithCache(item, () => dealPosition(watcher, item)),
            ),
        );
      },
    );

    onBeforeUnmount(() => {
      subscription.unsubscribe();
    });
  }
});

watch(currentGlobalKfLocation, () => {
  pos.value = [];
});

function handleClickRow(data: {
  event: MouseEvent;
  row: KungfuApi.PositionResolved;
  column: KfTradingDataTableHeaderConfig;
}) {
  const row = data.row;
  const { instrument_id, instrument_type, exchange_id } = row;
  const ensuredInstrument: KungfuApi.InstrumentResolved =
    getInstrumentByInstrumentPair(
      {
        instrument_id,
        instrument_type,
        exchange_id,
      },
      instruments.value,
    );

  triggerOrderBook(ensuredInstrument);

  const offset = resolveTriggerOffset(row);
  const extraOrderInput: ExtraOrderInput = {
    side: row.direction === 0 ? SideEnum.Sell : SideEnum.Buy,
    offset,
    volume: getPosClosableVolumeByOffset(row, offset),

    price: row.last_price || row.avg_open_price || 0,
    accountId: isTdStrategyCategory(currentGlobalKfLocation.value?.category)
      ? undefined
      : dealLocationUIDResolved(row.holder_uid),
  };
  triggerMakeOrder(ensuredInstrument, extraOrderInput);
}

function dealLocationUIDResolved(holderUID: number): string {
  return getIdByKfLocation(window.watcher.getLocation(holderUID));
}

function handleRequestPosition() {
  const res = window.watcher.requestPosition(window.watcher);
  if (res) {
    success(t('operation_success'));
  } else {
    error(t('operation_failed'));
  }
}
</script>
<template>
  <div class="kf-position__warp kf-translateZ">
    <KfDashboard @boardSizeChange="handleBodySizeChange">
      <template v-slot:title>
        <span v-if="currentGlobalKfLocation">
          <a-tag
            v-if="currentCategoryData"
            :color="currentCategoryData?.color || 'default'"
          >
            {{ currentCategoryData?.name }}
          </a-tag>
          <span class="name" v-if="currentGlobalKfLocation">
            {{ getCurrentGlobalKfLocationId(currentGlobalKfLocation) }}
          </span>
        </span>
      </template>
      <template v-slot:header>
        <KfDashboardItem>
          <a-input-search
            v-model:value="searchKeyword"
            :placeholder="$t('keyword_input')"
            style="width: 120px"
          />
        </KfDashboardItem>
        <KfDashboardItem>
          <a-button size="small" @click="handleRequestPosition">
            <template #icon>
              <ReloadOutlined style="font-size: 14px" />
            </template>
          </a-button>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-button
            size="small"
            @click="handleDownload('Position', currentGlobalKfLocation)"
          >
            <template #icon>
              <DownloadOutlined style="font-size: 14px" />
            </template>
          </a-button>
        </KfDashboardItem>
      </template>
      <KfTradingDataTable
        :columns="columns"
        :dataSource="tableData"
        keyField="uid_key"
        @clickCell="handleClickRow"
      >
        <template
          v-slot:default="{
            item,
            column,
          }: {
            item: KungfuApi.PositionResolved,
            column: KfTradingDataTableHeaderConfig,
          }"
        >
          <template v-if="column.dataIndex === 'instrument_id_resolved'">
            <span>
              {{ item.instrument_id_resolved }}
              <span
                v-if="globalSetting?.currency?.instrumentCurrency"
                style="color: #faad14"
              >
                {{ dealCurrency(item.currency).name }}
              </span>
            </span>
          </template>
          <template v-else-if="column.dataIndex === 'direction'">
            <span :class="`color-${dealDirection(item.direction).color}`">
              {{ dealDirection(item.direction).name }}
            </span>
          </template>
          <template v-else-if="column.dataIndex === 'yesterday_volume'">
            <KfBlinkNum
              :num="Number(item.yesterday_volume).toFixed(0)"
            ></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'today_volume'">
            <KfBlinkNum
              :num="Number(item.volume - item.yesterday_volume).toFixed(0)"
            ></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'volume'">
            <KfBlinkNum :num="Number(item.volume).toFixed(0)"></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'avg_open_price'">
            <KfBlinkNum :num="dealKfPrice(item.avg_open_price)"></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'last_price'">
            <KfBlinkNum
              :num="dealKfPrice(getPositionLastPrice(item))"
            ></KfBlinkNum>
          </template>
          <template v-else-if="column.dataIndex === 'unrealized_pnl'">
            <KfBlinkNum
              mode="compare-zero"
              :num="dealAssetPrice(item.unrealized_pnl)"
            ></KfBlinkNum>
          </template>
        </template>
      </KfTradingDataTable>
    </KfDashboard>
  </div>
</template>
<style lang="less">
.kf-position__warp {
  width: 100%;
  height: 100%;

  .kf-table__warp {
    width: 100%;
    height: 100%;

    .kf-trading-data-table {
      width: 100%;
      height: 100%;
    }
  }
}
</style>
