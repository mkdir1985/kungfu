<script lang="ts" setup>
import { sum } from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import { Empty } from 'ant-design-vue';
import { CaretUpOutlined, CaretDownOutlined } from '@ant-design/icons-vue';
import { filter } from 'rxjs';
import {
  computed,
  watch,
  getCurrentInstance,
  onBeforeMount,
  onMounted,
  ref,
  toRaw,
} from 'vue';

export interface API {
  selectedRowsMap: Record<string, KungfuApi.TradingDataItem>;
  isSelectAll: boolean;
  handleSelectRow: (
    isChecked: boolean,
    item: KungfuApi.TradingDataItem,
  ) => void;
  handleSelectAll: (isChecked: boolean) => void;
}

const props = withDefaults(
  defineProps<{
    dataSource: KungfuApi.TradingDataItem[];
    columns: KfTradingDataTableHeaderConfig[];
    keyField?: string;
    itemSize?: number;
    selectable?: boolean;
    selection?: KfTradingDataTableSelection; // 仅在 selectable 为 true 的时候生效
    customRowClass?: (row: KungfuApi.TradingDataItem) => string;
  }>(),
  {
    columns: () => [],
    dataSource: () => [],
    keyField: 'id',
    itemSize: 26,
    selectable: false,
    selection: () => ({}),
    customRowClass: () => '',
  },
);

defineEmits<{
  (
    e: 'dbclickRow',
    data: { event: MouseEvent; row: KungfuApi.TradingDataItem },
  ): void;
  (
    e: 'clickCell',
    data: {
      event: MouseEvent;
      row: KungfuApi.TradingDataItem;
      column: KfTradingDataTableHeaderConfig;
    },
  ): void;
  (
    e: 'clickCell',
    data: {
      event: MouseEvent;
      row: KungfuApi.TradingDataItem;
      column: KfTradingDataTableHeaderConfig;
    },
  ): void;
  (
    e: 'rightClickRow',
    data: { event: MouseEvent; row: KungfuApi.TradingDataItem },
  ): void;
}>();

const app = getCurrentInstance();
const simpleImage = Empty.PRESENTED_IMAGE_SIMPLE;
const kfScrollerTableBodyRef = ref();
const kfScrollerTableWidth = ref(0);
const dataSouceMap = ref<Record<string, KungfuApi.TradingDataItem>>({});
let allRowKeyFieldTrue: Record<string, boolean> = {};
let allRowKeyFieldFalse: Record<string, boolean> = {};
const isSelectAll = ref(false);
const selectAllIndeterminate = ref(false);
const selectedRowKeyFieldValues = ref<Record<string, boolean>>({});
const selectedRowsMap = ref<Record<string, KungfuApi.TradingDataItem>>({});
let clickTimer: number | undefined;

const headerWidth = computed(() => {
  const widths: KfTradingDataTableHeaderConfig[] = []; //column use with
  const flexs: KfTradingDataTableHeaderConfig[] = []; //column use flex

  props.columns.forEach((item) => {
    if (item.width !== undefined) {
      widths.push(item);
    } else {
      flexs.push(item);
    }
  });

  const flexWidthUnits = sum(flexs.map((item) => item.flex || 1));
  const widthForFlex =
    kfScrollerTableWidth.value -
    (widths.length ? sum(widths.map((item) => item.width || 0)) : 0);
  const unit = widthForFlex / flexWidthUnits;

  return [...widths, ...flexs].reduce((collection, item) => {
    collection[item.dataIndex] = item.width
      ? item.width + 'px'
      : unit * (item.flex || 1) + 'px';
    return collection;
  }, {} as Record<string, string>);
});

const tableCellHeight = computed(() => `${props.itemSize}px`);

watch(
  () => props.dataSource,
  (newDataSource) => {
    dataSouceMap.value = {};
    allRowKeyFieldTrue = {};
    allRowKeyFieldFalse = {};

    const tempSelectedValues = {};
    const tempSelectedRows = {};

    newDataSource.forEach((item) => {
      const key = `${item[props.keyField]}`;
      dataSouceMap.value[key] = item;
      allRowKeyFieldTrue[key] = true;
      allRowKeyFieldFalse[key] = false;

      if (key in selectedRowKeyFieldValues.value) {
        tempSelectedValues[key] = selectedRowKeyFieldValues.value[key];
        tempSelectedRows[key] = selectedRowsMap.value[key];
      }
    });

    selectedRowKeyFieldValues.value = tempSelectedValues;
    selectedRowsMap.value = tempSelectedRows;
  },
  { immediate: true },
);

onMounted(() => {
  if (kfScrollerTableBodyRef.value) {
    kfScrollerTableWidth.value = kfScrollerTableBodyRef.value.clientWidth;
  }

  if (app?.proxy) {
    const subscription = app?.proxy.$globalBus
      .pipe(filter((e: KfEvent.KfBusEvent) => e.tag === 'resize'))
      .subscribe(() => {
        if (kfScrollerTableBodyRef.value) {
          kfScrollerTableWidth.value = kfScrollerTableBodyRef.value.clientWidth;
        }
      });

    onBeforeMount(() => {
      subscription.unsubscribe();
    });
  }
});

function getHeaderWidth(column: KfTradingDataTableHeaderConfig): string {
  const headerWidthByCalc = headerWidth.value[column.dataIndex];
  const columnWidth = +(column?.width || 0);

  if ((parseInt(headerWidthByCalc) <= 0 || !headerWidthByCalc) && columnWidth) {
    return columnWidth + 'px';
  } else {
    return headerWidthByCalc.toString();
  }
}

function handleDbClickRow(e: MouseEvent, row: KungfuApi.TradingDataItem): void {
  app && app.emit('dbclickRow', { event: e, row });
  clickTimer && clearTimeout(clickTimer);
}

function handleClickCell(
  e: MouseEvent,
  row: KungfuApi.TradingDataItem,
  column: KfTradingDataTableHeaderConfig,
): void {
  clickTimer && clearTimeout(clickTimer);
  clickTimer = +setTimeout(() => {
    app && app.emit('clickCell', { event: e, row, column });
  }, 300);
}

function handleMousedown(e: MouseEvent, row: KungfuApi.TradingDataItem): void {
  if (e.button === 2) {
    app && app.emit('rightClickRow', { event: e, row });
  }
}

const currentSorterIndex = ref<string>('');
const currentSorterOrder = ref<'' | 'ascend' | 'descend'>('');
let currentSorterFunction: ((a: any, b: any) => number) | undefined = undefined;
const dataSourceResolved = computed(() => {
  if (
    currentSorterIndex.value &&
    currentSorterFunction &&
    currentSorterOrder.value !== ''
  ) {
    if (currentSorterOrder.value === 'ascend') {
      return props.dataSource.slice(0).sort(currentSorterFunction);
    } else {
      return props.dataSource.slice(0).sort(currentSorterFunction).reverse();
    }
  }
  return props.dataSource;
});

function handleSort(
  dataIndex: string,
  sorter: undefined | ((a: any, b: any) => number),
): void {
  if (!sorter || !dataIndex) {
    return;
  }

  currentSorterFunction = sorter;

  if (currentSorterIndex.value) {
    if (dataIndex === currentSorterIndex.value) {
      if (currentSorterOrder.value === '') {
        currentSorterOrder.value = 'ascend';
      } else if (currentSorterOrder.value === 'ascend') {
        currentSorterOrder.value = 'descend';
      } else {
        currentSorterOrder.value = '';
      }
    } else {
      currentSorterIndex.value = dataIndex;
      currentSorterOrder.value = 'ascend';
    }
  } else {
    currentSorterIndex.value = dataIndex;
    currentSorterOrder.value = 'ascend';
  }
}

function handleSelectRow(isChecked: boolean, item: KungfuApi.TradingDataItem) {
  if (!props.selectable) return;

  const key = item[props.keyField];

  selectedRowKeyFieldValues.value[key] = isChecked;

  if (isChecked) {
    selectedRowsMap.value[key] = toRaw(dataSouceMap.value[key]);
  } else {
    delete selectedRowsMap.value[key];
  }
}

function handleSelectAll(isChecked: boolean) {
  if (!props.selectable) return;

  const allSelected = Object.assign({}, allRowKeyFieldTrue);
  const allUnSelected = Object.assign({}, allRowKeyFieldFalse);
  const allRowsMap = Object.assign({}, toRaw(dataSouceMap.value));

  selectedRowKeyFieldValues.value = isChecked ? allSelected : allUnSelected;
  selectedRowsMap.value = isChecked ? allRowsMap : {};
}

watch(
  () => selectedRowKeyFieldValues.value,
  (val) => {
    if (!props.selectable) return;

    const allRowLength = props.dataSource.length;
    if (!allRowLength) return;

    const selectedRowLength = Object.values(val).filter((item) => item).length;

    selectAllIndeterminate.value =
      !!selectedRowLength && selectedRowLength < allRowLength;
    isSelectAll.value = selectedRowLength === allRowLength;
  },
  {
    deep: true,
  },
);

defineExpose({
  selectedRowsMap,
  isSelectAll,
  handleSelectRow,
  handleSelectAll,
});
</script>
<template>
  <div class="kf-table">
    <ul class="kf-table-header kf-table-row">
      <li
        v-if="selectable"
        class="kf-table-cell kf-table-select-cell"
        style="width: 36px; flex-basis: 36px"
        :title="$t('select_all')"
      >
        <a-checkbox
          v-model:checked="isSelectAll"
          :indeterminate="selectAllIndeterminate"
          @change="handleSelectAll(!!$event.target.checked)"
        />
      </li>
      <li
        v-for="column in columns"
        :key="column.dataIndex"
        :class="['kf-table-cell', column.type]"
        :title="column.name"
        :style="{
          'max-width': getHeaderWidth(column),
        }"
        @click.stop="handleSort(column.dataIndex, column.sorter)"
      >
        <span class="name">{{ column.name }}</span>
        <span v-if="column.sorter" class="sort-btn">
          <CaretUpOutlined
            style="color: #bfbfbf; font-size: 11px"
            :class="{
              active:
                column.dataIndex === currentSorterIndex &&
                currentSorterOrder === 'ascend',
            }"
          ></CaretUpOutlined>
          <CaretDownOutlined
            style="color: #bfbfbf; font-size: 11px; margin-top: -1px"
            :class="{
              active:
                column.dataIndex === currentSorterIndex &&
                currentSorterOrder === 'descend',
            }"
          ></CaretDownOutlined>
        </span>
      </li>
    </ul>
    <div ref="kfScrollerTableBodyRef" class="kf-table-body">
      <RecycleScroller
        v-if="dataSourceResolved && dataSourceResolved.length"
        class="kf-table-scroller"
        :items="dataSourceResolved"
        :item-size="Number(itemSize)"
        :key-field="keyField"
        :buffer="100"
      >
        <template #default="{ item }: { item: any }">
          <ul
            :class="['kf-table-row', customRowClass?.(item) || '']"
            @dblclick="handleDbClickRow($event, item)"
            @mousedown="handleMousedown($event, item)"
          >
            <li
              v-if="selectable"
              class="kf-table-cell kf-table-select-cell"
              :style="{
                width: '36px',
                flexBasis: '36px',
                height: tableCellHeight,
                lineHeight: tableCellHeight,
              }"
            >
              <a-checkbox
                v-model:checked="selectedRowKeyFieldValues[item[keyField]]"
                :disabled="selection[item[keyField]]?.disabled ?? false"
                @change="handleSelectRow(!!$event.target.checked, item)"
              ></a-checkbox>
            </li>
            <li
              v-for="column in columns"
              :key="`${column.dataIndex}_${item[keyField as keyof KungfuApi.TradingDataItem]}`"
              :class="['kf-table-cell', column.type]"
              :style="{
                'max-width': getHeaderWidth(column),
                height: tableCellHeight,
                lineHeight: tableCellHeight,
              }"
              :title="item[column.dataIndex]"
              @click.stop="handleClickCell($event, item, column)"
            >
              <slot :item="item" :column="column">
                <span>
                  {{
                    item[column.dataIndex as keyof KungfuApi.TradingDataItem]
                  }}
                </span>
              </slot>
            </li>
          </ul>
        </template>
      </RecycleScroller>
      <a-empty v-else :image="simpleImage"></a-empty>
    </div>
  </div>
</template>
<style lang="less">
.kf-table {
  display: flex;
  flex-direction: column;
  height: 100%;
  position: relative;

  .ant-empty {
    height: auto;
    margin-top: 48px;

    .ant-empty-image {
      height: auto;
    }

    .ant-empty-description {
      color: @input-placeholder-color;
    }
  }

  .kf-table-header {
    display: flex;
    flex-direction: row;
    justify-content: flex-start;
    height: 36px;
    line-height: 36px;
    background: @table-header-bg;
    white-space: nowrap;
    box-sizing: border-box;
    margin-bottom: 4px;

    .kf-table-cell {
      height: 36px;
      line-height: 36px;
      display: flex;
      user-select: none;
      position: relative;

      .kf-table-row:hover {
        background: @table-header-bg;
      }

      .name {
        flex: 1;
      }

      .sort-btn {
        transition: color 0.3s;
        display: flex;
        flex-direction: column;
        align-items: center;
        justify-content: center;

        .anticon {
          &.active {
            color: @primary-color !important;
          }

          &.anticon-caret-down {
            margin-top: -3px !important;
          }
        }
      }

      &::after {
        content: '';
        width: 0;
        height: 60%;
        position: absolute;
        right: 0;
        top: 50%;
        transform: translateY(-50%);
        border-left: 1px solid @table-header-cell-split-color;
      }
    }
  }

  .kf-table-body {
    width: 100%;
    transform: translateZ(0);
    position: absolute;
    top: 36px;
    height: calc(100% - 36px);

    .kf-table-scroller {
      height: 100%;
      width: 100%;
      overflow-y: overlay;
    }
  }

  .kf-table-row {
    display: flex;
    flex-direction: row;
    justify-content: flex-start;
    cursor: pointer;
    list-style: none;
    margin: 0;
  }

  .kf-table-row:hover {
    background: @table-row-hover-bg;
  }

  .kf-table-cell {
    padding: 0 6px;
    box-sizing: border-box;
    word-wrap: break-word;
    flex: 1;
    font-size: 12px;
    user-select: text;
    text-align: left;
    position: relative;
    white-space: nowrap;
    text-overflow: unset;
    overflow: hidden;

    &.number {
      text-align: right;
    }
  }

  .kf-table-select-cell {
    flex-grow: 0;
    flex-shrink: 0;
    display: flex;
    align-items: center;
    justify-content: center;
  }
}
</style>
