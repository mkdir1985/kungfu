import { LedgerCategoryEnum } from '@kungfu-trader/kungfu-js-api/typings/enums';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { DealTradingDataGetter } from '@kungfu-trader/kungfu-js-api/hooks/dealTradingDataHook';
import { getTradingDataSortKey } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
const { t } = VueI18n.global;

const buildSorter =
  (dataIndex: keyof KungfuApi.PositionResolved) =>
  (a: KungfuApi.PositionResolved, b: KungfuApi.PositionResolved) =>
    (+Number(a[dataIndex]) || 0) - (+Number(b[dataIndex]) || 0);

const buildStrSorter =
  (dataIndex: keyof KungfuApi.PositionResolved) =>
  (a: KungfuApi.PositionResolved, b: KungfuApi.PositionResolved) =>
    a[dataIndex].toString().localeCompare(b[dataIndex].toString());

export const getColumns = (): KfTradingDataTableHeaderConfig[] => [
  {
    type: 'string',
    name: t('posGlobalConfig.instrument_id'),
    dataIndex: 'instrument_id',
    width: 260,
    sorter: buildStrSorter('instrument_id'),
  },
  {
    name: '',
    dataIndex: 'direction',
    width: 50,
  },
  {
    type: 'number',
    name: t('posGlobalConfig.yesterday_volume'),
    dataIndex: 'yesterday_volume',
    width: 80,
    sorter: buildSorter('yesterday_volume'),
  },
  {
    type: 'number',
    name: t('posGlobalConfig.today_volume'),
    dataIndex: 'today_volume',
    width: 80,
    sorter: (a: KungfuApi.Position, b: KungfuApi.Position) => {
      const deltaA = a.volume - a.yesterday_volume;
      const deltaB = b.volume - b.yesterday_volume;
      return +Number(deltaA) - +Number(deltaB);
    },
  },
  {
    type: 'number',
    name: t('posGlobalConfig.sum_volume'),
    dataIndex: 'volume',
    width: 80,
    sorter: buildSorter('volume'),
  },
  {
    type: 'number',
    name: t('posGlobalConfig.avg_open_price'),
    dataIndex: 'avg_open_price',
    width: 110,
    sorter: buildSorter('avg_open_price'),
  },
  {
    type: 'number',
    name: t('posGlobalConfig.last_price'),
    dataIndex: 'last_price',
    width: 110,
    sorter: buildSorter('last_price'),
  },
  {
    type: 'number',
    name: t('posGlobalConfig.unrealized_pnl'),
    dataIndex: 'unrealized_pnl',
    width: 110,
    sorter: buildSorter('unrealized_pnl'),
  },
];

const orderSortKey = getTradingDataSortKey('Order');
const tradeSortKey = getTradingDataSortKey('Trade');
const positionSortKey = getTradingDataSortKey('Position');
export const categoryRegisterConfig: DealTradingDataGetter = {
  category: 'globalPos',
  commonData: {
    name: t('PosGlobal'),
    color: 'pink',
  },
  order: {
    getter(watcher, orders, kfLocation) {
      const { group, name } = kfLocation;
      return orders
        .filter('exchange_id', group)
        .filter('instrument_id', name)
        .sort(orderSortKey);
    },
  },
  trade: {
    getter(watcher, trades, kfLocation) {
      const { group, name } = kfLocation;
      return trades
        .filter('exchange_id', group)
        .filter('instrument_id', name)
        .sort(tradeSortKey);
    },
  },
  position: {
    getter(watcher, position, kfLocation) {
      const { group, name, direction } =
        kfLocation as KungfuApi.KfExtraLocation;
      return position
        .nofilter('volume', BigInt(0))
        .filter('ledger_category', LedgerCategoryEnum.td)
        .filter('exchange_id', group)
        .filter('instrument_id', name)
        .filter('direction', direction)
        .sort(positionSortKey)
        .reverse();
    },
  },
};
