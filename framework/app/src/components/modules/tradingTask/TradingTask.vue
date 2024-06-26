<script lang="ts" setup>
import {
  handleOpenLogview,
  useDashboardBodySize,
  useTableSearchKeyword,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import { computed, inject, ref } from 'vue';
import minimist from 'minimist';

import KfDashboard from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboard.vue';
import KfDashboardItem from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfDashboardItem.vue';
import KfSetExtensionModal from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfSetExtensionModal.vue';
import {
  FileTextOutlined,
  SettingOutlined,
  DeleteOutlined,
} from '@ant-design/icons-vue';
import { getColumns } from './config';
import path from 'path';
import {
  getIfProcessRunning,
  getIfProcessStopping,
  getStrategyKfLocationByProcessId,
  fromProcessArgsToKfConfigItems,
  kfConfigItemsToArgsByPrimaryForShow,
  dealTradingTaskName,
  getTaskListFromProcessStatusData,
} from '@kungfu-trader/kungfu-js-api/utils/busiUtils';
import {
  graceStopProcess,
  Pm2ProcessStatusDetail,
  Pm2ProcessStatusDetailResolved,
  startTask,
} from '@kungfu-trader/kungfu-js-api/utils/processUtils';
import KfProcessStatus from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfProcessStatus.vue';
import {
  playSound,
  useAddUpdateRemoveKfConfig,
  useCurrentGlobalKfLocation,
  useExtConfigsRelated,
  useProcessStatusDetailData,
} from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/actionsUtils';
import { messagePrompt } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { useTradingTask } from './utils';

import { ProcessStatusTypes } from '@kungfu-trader/kungfu-js-api/typings/enums';
import { useGlobalStore } from '@kungfu-trader/kungfu-app/src/renderer/pages/index/store/global';
import { storeToRefs } from 'pinia';
import { BuiltinComponentInjectKeysMap } from '@kungfu-trader/kungfu-app/src/renderer/assets/configs/symbols';

const { t } = VueI18n.global;
const columns = getColumns();
const { success, error } = messagePrompt();
const { extConfigs } = useExtConfigsRelated();
const { dashboardBodyHeight, handleBodySizeChange } = useDashboardBodySize();
const { processStatusData, processStatusDetailData, getStrategyStatusName } =
  useProcessStatusDetailData();
const { globalFormState } = storeToRefs(useGlobalStore());
const tradingTaskPropsInject = inject(
  BuiltinComponentInjectKeysMap.TradingTask,
  {},
);

const { handleOpenSetTradingTaskModal } = useTradingTask();
const { handleRemoveKfConfig } = useAddUpdateRemoveKfConfig();

const setExtensionModalVisible = ref<boolean>(false);

const taskTypeKeys = computed(() => {
  return Object.keys(extConfigs.value['strategy'] || {});
});
const taskList = computed(() => {
  const taskPrefixs = taskTypeKeys.value.map((item) => {
    return `strategy_${item}`;
  });
  const tasksResolved: Pm2ProcessStatusDetailResolved[] =
    getTaskListFromProcessStatusData(
      taskPrefixs,
      processStatusDetailData.value,
      tradingTaskPropsInject?.taskSorter,
    ).map((item) => {
      return {
        ...item,
        name_resolved: dealTradingTaskName(
          item.name as string,
          extConfigs.value,
        ),
      };
    });

  if (tradingTaskPropsInject?.taskFilter) {
    return tasksResolved.filter((item) =>
      tradingTaskPropsInject?.taskFilter?.(item),
    );
  }

  return tasksResolved;
});
const { searchKeyword, tableData } =
  useTableSearchKeyword<Pm2ProcessStatusDetailResolved>(taskList, [
    'name',
    'args',
    'name_resolved',
  ]);

const { dealRowClassName, setCurrentGlobalKfLocation } =
  useCurrentGlobalKfLocation(window.watcher);

const { uiExtConfigs } = useExtConfigsRelated();

const TradingTaskViewComponentConfigs = computed(() => {
  return Object.keys(uiExtConfigs.value)
    .filter((key) => uiExtConfigs.value[key].position === 'trading_task_view')
    .map((key) => {
      return {
        ...uiExtConfigs.value[key],
        key,
      };
    });
});

function handleOpenSetTaskDialog() {
  setExtensionModalVisible.value = true;
}

function handleSwitchProcessStatusResolved(
  checked: boolean,
  event: MouseEvent,
  record: Pm2ProcessStatusDetail,
) {
  event.stopPropagation();

  const taskLocation = getStrategyKfLocationByProcessId(record?.name || '');
  if (!taskLocation) {
    error(`${record.name} ${t('tradingTaskConfig.illegal_process_id')}`);
    return;
  }

  if (!checked) {
    return graceStopProcess(
      window.watcher,
      taskLocation,
      processStatusData.value,
    )
      .then(() => {
        success();
      })
      .catch((err: Error) => {
        error(err.message || t('operation_failed'));
      });
  }

  const extKey = taskLocation.group;
  const extConfig: KungfuApi.KfExtConfig = (extConfigs.value['strategy'] || {})[
    extKey
  ];

  if (!extConfig) {
    error(`${extKey} ${t('tradingTaskConfig.plugin_inexistence')}`);
    return;
  }

  if (!extConfig.extPath) {
    error(
      `${t('tradingTaskConfig.configuration_inexistence')} ${extConfig?.name}`,
    );
    return;
  }

  const soPath = path.join(extConfig.extPath, extKey);
  const args = minimist(record.args as string[])['a'] || '';
  const configSettings = parseTaskSettingsFromEnv(
    record.config_settings || '[]',
  );

  return startTask(taskLocation, soPath, args, configSettings)
    .catch((err: Error) => error(err.message))
    .then(() => {
      success();
    })
    .catch((err: Error) => {
      error(err.message || t('operation_failed'));
    });
}

function handleOpenLogviewResolved(record: Pm2ProcessStatusDetail) {
  const taskLocation = getStrategyKfLocationByProcessId(record?.name || '');
  if (!taskLocation) {
    error(`${record.name} ${t('tradingTaskConfig.illegal_process_id')}`);
    return;
  }
  handleOpenLogview(taskLocation);
}

function handleRemoveTask(record: Pm2ProcessStatusDetail) {
  const taskLocation = getStrategyKfLocationByProcessId(record?.name || '');
  if (!taskLocation) {
    error(`${record.name} ${t('tradingTaskConfig.illegal_process_id')}`);
    return;
  }

  return handleRemoveKfConfig(
    window.watcher,
    taskLocation,
    processStatusData.value,
  )
    .then(() => {
      success();
    })
    .catch((err) => {
      error(err.message || t('operation_failed'));
    });
}

function customRowResolved(record: Pm2ProcessStatusDetail) {
  const taskLocation = getStrategyKfLocationByProcessId(record?.name || '');
  if (!taskLocation) {
    error(`${record.name} ${t('tradingTaskConfig.illegal_process_id')}`);
    return;
  }
  const locationResolved: KungfuApi.KfExtraLocation =
    resolveKfLocation(taskLocation);

  return {
    onClick: () => {
      setCurrentGlobalKfLocation(locationResolved);
    },
  };
}

function dealRowClassNameResolved(record: Pm2ProcessStatusDetail): string {
  const taskLocation = getStrategyKfLocationByProcessId(record?.name || '');
  if (!taskLocation) {
    error(`${record.name} ${t('tradingTaskConfig.illegal_process_id')}`);
    return '';
  }
  const locationResolved: KungfuApi.KfExtraLocation =
    resolveKfLocation(taskLocation);

  return dealRowClassName(locationResolved);
}

function resolveKfLocation(
  taskLocation: KungfuApi.KfLocation,
): KungfuApi.KfExtraLocation {
  const locationResolved: KungfuApi.KfExtraLocation = {
    category: 'strategy',
    group: taskLocation?.group || '',
    name: taskLocation?.name || '',
    mode: 'live',
  };
  return locationResolved;
}

const taskArgsData: Record<string, string> = {};

function resolveArgs(record: Pm2ProcessStatusDetail) {
  const name = record?.name || '';
  if (taskArgsData[name]) {
    return taskArgsData[name];
  }

  const configSettings = parseTaskSettingsFromEnv(record.config_settings);
  const formState = fromProcessArgsToKfConfigItems(record.args || []);
  const argsForShow = kfConfigItemsToArgsByPrimaryForShow(
    configSettings,
    formState,
  );
  taskArgsData[name] = argsForShow;
  return argsForShow;
}

function parseTaskSettingsFromEnv(configSettingsEnv = '[]') {
  let configSettings: KungfuApi.KfConfigItem[] = [];

  try {
    configSettings = JSON.parse(configSettingsEnv) as KungfuApi.KfConfigItem[];
  } catch (err) {
    console.error((<Error>err).message);
  }
  return configSettings;
}

const strategyStateMap: Record<string, string> = {};

function getProcessStatusName(
  record: Pm2ProcessStatusDetail,
): ProcessStatusTypes | undefined {
  const taskLocation = getStrategyKfLocationByProcessId(record?.name || '');
  if (!taskLocation) {
    return;
  }

  const newState = getStrategyStatusName(taskLocation);
  const oldState = strategyStateMap[record?.name || ''];

  const isOldWarn = oldState === 'Warn' || oldState === 'Error';
  const isNewWarn = newState === 'Warn' || newState === 'Error';
  const oldNotEqualNew = isOldWarn && isNewWarn && oldState !== newState;

  if ((!isOldWarn && isNewWarn) || oldNotEqualNew) {
    playSound('warn');
  }

  strategyStateMap[record?.name || ''] = newState || '';

  return newState;
}
</script>

<template>
  <div class="kf-task-strategy__wrap kf-translateZ">
    <KfDashboard @boardSizeChange="handleBodySizeChange">
      <template v-slot:header>
        <KfDashboardItem>
          <a-input-search
            v-model:value="searchKeyword"
            :placeholder="$t('keyword_input')"
            style="width: 120px"
          />
        </KfDashboardItem>
        <KfDashboardItem v-for="config in TradingTaskViewComponentConfigs">
          <component :is="config.key"></component>
        </KfDashboardItem>
        <KfDashboardItem>
          <a-button
            size="small"
            type="primary"
            @click="handleOpenSetTaskDialog"
          >
            {{ $t('tradingTaskConfig.add_task') }}
          </a-button>
        </KfDashboardItem>
      </template>
      <a-table
        class="kf-ant-table"
        :columns="columns"
        :data-source="tableData"
        size="small"
        :pagination="false"
        :rowClassName="dealRowClassNameResolved"
        :customRow="customRowResolved"
        :scroll="{ y: dashboardBodyHeight - 4 }"
        :defaultExpandAllRows="true"
        :emptyText="$t('empty_text')"
      >
        <template #bodyCell="{ column, record }">
          <template v-if="column.dataIndex === 'name'">
            {{ record.name_resolved }}
          </template>
          <template v-else-if="column.dataIndex === 'processStatus'">
            <a-switch
              size="small"
              :checked="getIfProcessRunning(processStatusData, record.name)"
              :loading="getIfProcessStopping(processStatusData, record.name)"
              @click="(checked: boolean, Event: MouseEvent) => handleSwitchProcessStatusResolved(checked, Event, record)"
            ></a-switch>
          </template>
          <template v-if="column.dataIndex === 'args'">
            {{ resolveArgs(record) }}
          </template>
          <template v-else-if="column.dataIndex === 'stateStatus'">
            <KfProcessStatus
              :statusName="getProcessStatusName(record)"
            ></KfProcessStatus>
          </template>
          <template v-else-if="column.dataIndex === 'actions'">
            <div class="kf-actions__warp">
              <FileTextOutlined
                style="font-size: 12px"
                @click.stop="handleOpenLogviewResolved(record)"
              />
              <SettingOutlined
                style="font-size: 12px"
                @click.stop="
                  handleOpenSetTradingTaskModal(
                    'update',
                    getStrategyKfLocationByProcessId(record?.name || '')
                      ?.group || '',
                    fromProcessArgsToKfConfigItems(record.args),
                  )
                "
              />
              <DeleteOutlined
                style="font-size: 12px"
                @click.stop="handleRemoveTask(record)"
              />
            </div>
          </template>
        </template>
      </a-table>
    </KfDashboard>
    <KfSetExtensionModal
      v-if="setExtensionModalVisible"
      v-model:visible="setExtensionModalVisible"
      extensionType="strategy"
      :ext-filter="tradingTaskPropsInject?.strategyFilter"
      @confirm="handleOpenSetTradingTaskModal('add', $event, globalFormState)"
    ></KfSetExtensionModal>
  </div>
</template>
<style lang="less">
.kf-task-strategy__wrap {
  height: 100%;
}
</style>
