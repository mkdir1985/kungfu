<template>
  <KfDragRow :id="boardId" v-if="direction === h">
    <template
      v-for="childBoardId in boardInfo.children || []"
      :key="childBoardId"
    >
      <KfRowColIter
        :board-id="childBoardId"
        :closable="closable"
      ></KfRowColIter>
    </template>
    <template v-if="contents.length">
      <a-tabs
        size="small"
        :class="{
          [classNameForTabDrag]: true,
          'is-dragging': isBoardDragging,
        }"
        :active-key="boardInfo.current"
        style="height: 100%; width: 100%"
        :type="closable ? 'editable-card' : 'card'"
        :tab-bar-style="{ margin: 0 }"
        @edit="(targetKey, action) => hanldeEdit(boardId, targetKey, action)"
        @tabClick="handleClickTab"
        @dragenter="handleDragEnter"
        @dragover="handleDragOver"
        @dragleave="handleDragLeave"
        @drop="handleDrop"
      >
        <a-tab-pane v-for="content in contents" :key="content">
          <template v-slot:tab>
            <div
              class="kf-tab-header"
              draggable="true"
              @dragstart="handleDragStart(content)"
              @dragend="handleDragEnd"
            >
              {{ getBoardNameByLanguage(content) }}
            </div>
          </template>
          <a-card style="width: 100%; height: 100%">
            <component
              v-if="hasComponent(content) && content === boardInfo.current"
              :is="content"
              :id="content"
            ></component>
            <KfNoData
              v-else
              :txt="`${getBoardNameByLanguage(content)} ${$t(
                'component_error',
              )}`"
            ></KfNoData>
          </a-card>
        </a-tab-pane>
      </a-tabs>
    </template>
  </KfDragRow>
  <KfDragCol :id="boardId" v-else-if="direction === v">
    <template
      v-for="childBoardId in boardInfo.children || []"
      :key="childBoardId"
    >
      <KfRowColIter
        :board-id="childBoardId"
        :closable="closable"
      ></KfRowColIter>
    </template>
    <template v-if="contents.length">
      <a-tabs
        size="small"
        :class="{
          [classNameForTabDrag]: true,
          'is-dragging': isBoardDragging,
        }"
        :active-key="boardInfo.current"
        style="height: 100%; width: 100%"
        :type="closable ? 'editable-card' : 'card'"
        :tab-bar-style="{ margin: 0 }"
        @edit="(targetKey, action) => hanldeEdit(boardId, targetKey, action)"
        @tabClick="handleClickTab"
        @dragenter="handleDragEnter"
        @dragover="handleDragOver"
        @dragleave="handleDragLeave"
        @drop="handleDrop"
      >
        <a-tab-pane v-for="content in contents" :key="content">
          <template v-slot:tab>
            <div
              class="kf-tab-header"
              draggable="true"
              @dragstart="handleDragStart(content)"
              @dragend="handleDragEnd"
            >
              {{ getBoardNameByLanguage(content) }}
            </div>
          </template>
          <a-card style="width: 100%; height: 100%">
            <component
              v-if="hasComponent(content) && content === boardInfo.current"
              :is="content"
              :id="content"
            ></component>
            <KfNoData
              v-else
              :txt="`${getBoardNameByLanguage(content)} ${$t(
                'component_error',
              )}`"
            ></KfNoData>
          </a-card>
        </a-tab-pane>
      </a-tabs>
    </template>
  </KfDragCol>
</template>

<script lang="ts">
import { defineComponent, PropType, reactive, toRefs } from 'vue';
import { storeToRefs } from 'pinia';
import {
  KfLayoutDirection,
  KfLayoutTargetDirectionClassName,
} from '@kungfu-trader/kungfu-app/src/typings/enums';

import KfDragRow from '@kungfu-trader/kungfu-app/src/renderer/components/layout/KfDragRow.vue';
import KfDragCol from '@kungfu-trader/kungfu-app/src/renderer/components/layout/KfDragCol.vue';
import KfNoData from '@kungfu-trader/kungfu-app/src/renderer/components/public/KfNoData.vue';

import { useGlobalStore } from '@kungfu-trader/kungfu-app/src/renderer/pages/index/store/global';
import VueI18n, { useLanguage } from '@kungfu-trader/kungfu-js-api/language';

const { t } = VueI18n.global;

interface KfRowColIterData {
  h: KfLayoutDirection;
  v: KfLayoutDirection;
  classNameForTabDrag: KfLayoutTargetDirectionClassName;
  dragEnterBoxWidth14: number;
  dragEnterBoxWidth34: number;
  dragEnterBoxHeight14: number;
  dragEnterBoxHeight34: number;
}

export default defineComponent({
  name: 'KfRowColIter',

  components: {
    KfDragCol,
    KfDragRow,
    KfNoData,
  },

  props: {
    boardId: {
      type: Number as PropType<number>,
      default: 0,
    },

    closable: {
      type: Boolean as PropType<boolean>,
      default: false,
    },
  },

  setup() {
    const { boardsMap, dragedContentData, isBoardDragging } = storeToRefs(
      useGlobalStore(),
    );
    const { isLanguageKeyAvailable } = useLanguage();

    const getBoardNameByLanguage = (contentId: string) =>
      isLanguageKeyAvailable(contentId) ? t(contentId) : contentId;

    const {
      setBoardsMapAttrById,
      removeBoardByContentId,
      setDragedContentData,
      afterDragMoveBoard,
      markIsBoardDragging,
    } = useGlobalStore();

    const rowColIterData = reactive<KfRowColIterData>({
      h: KfLayoutDirection.h,
      v: KfLayoutDirection.v,
      classNameForTabDrag: KfLayoutTargetDirectionClassName.unset,
      dragEnterBoxWidth14: 0,
      dragEnterBoxWidth34: 0,
      dragEnterBoxHeight14: 0,
      dragEnterBoxHeight34: 0,
    });

    return {
      ...toRefs(rowColIterData),
      boardsMap,
      dragedContentData,
      isBoardDragging,

      setBoardsMapAttrById,
      removeBoardByContentId,
      setDragedContentData,
      afterDragMoveBoard,
      markIsBoardDragging,

      getBoardNameByLanguage,
    };
  },

  computed: {
    boardInfo(): KfLayout.BoardInfo {
      return this.boardsMap[this.boardId] || {};
    },

    children(): number[] {
      return this.boardInfo.children || [];
    },

    contents(): string[] {
      return this.boardInfo.contents || [];
    },

    direction(): string {
      return this.boardInfo.direction || '';
    },
  },

  methods: {
    handleDragStart(contentId: KfLayout.ContentId) {
      this.setDragedContentData(this.boardId, contentId);
      this.markIsBoardDragging(true);
    },

    handleDragEnter(e: DragEvent) {
      const target: HTMLElement | null = e.target as HTMLElement | null;
      const width: number = target?.clientWidth || 0;
      const height: number = target?.clientHeight || 0;

      this.dragEnterBoxWidth14 = width / 4;
      this.dragEnterBoxWidth34 = (width * 3) / 4;
      this.dragEnterBoxHeight14 = height / 4;
      this.dragEnterBoxHeight34 = (height * 3) / 4;
    },

    handleDragOver(e: DragEvent) {
      const { offsetX, offsetY } = e;

      if (offsetX < this.dragEnterBoxWidth14) {
        this.classNameForTabDrag = KfLayoutTargetDirectionClassName.left;
      } else if (offsetX > this.dragEnterBoxWidth34) {
        this.classNameForTabDrag = KfLayoutTargetDirectionClassName.right;
      } else if (offsetY < this.dragEnterBoxHeight14) {
        this.classNameForTabDrag = KfLayoutTargetDirectionClassName.top;
      } else if (offsetY > this.dragEnterBoxHeight34) {
        this.classNameForTabDrag = KfLayoutTargetDirectionClassName.bottom;
      } else {
        this.classNameForTabDrag = KfLayoutTargetDirectionClassName.center;
      }

      e.preventDefault();
    },

    handleDragLeave() {
      this.classNameForTabDrag = KfLayoutTargetDirectionClassName.unset;
    },

    handleDragEnd() {
      this.clearState();
      this.markIsBoardDragging(false);
      this.$globalBus.next({
        tag: 'resize',
      } as KfEvent.ResizeEvent);
    },

    handleDrop() {
      this.afterDragMoveBoard(
        this.dragedContentData,
        this.boardId,
        this.classNameForTabDrag,
      );
      this.clearState();
    },

    hanldeEdit(
      boardId: number,
      targetContentId: KfLayout.BoardInfo['current'],
      action: 'add' | 'remove',
    ) {
      if (action === 'remove') {
        const targetBoardId = this.boardId;
        this.removeBoardByContentId(targetBoardId, targetContentId || '');
      } else {
        this.$globalBus.next({
          tag: 'addBoard',
          boardId,
        });
      }
    },

    handleClickTab(e: KfLayout.BoardInfo['current']) {
      this.setBoardsMapAttrById(this.boardId, 'current', e?.toString());
    },

    clearState() {
      this.classNameForTabDrag = KfLayoutTargetDirectionClassName.unset;
      this.dragEnterBoxWidth14 = 0;
      this.dragEnterBoxWidth34 = 0;
      this.dragEnterBoxHeight14 = 0;
      this.dragEnterBoxHeight34 = 0;
      this.setDragedContentData(-1, '');
    },

    hasComponent(cname: string) {
      return !!this._.appContext.components[cname];
    },
  },
});
</script>
<style lang="less">
.ant-tabs.ant-tabs-card.ant-tabs-small {
  > .ant-tabs-nav .ant-tabs-tab {
    padding: 0;
    position: relative;

    .kf-tab-header {
      padding: 6px 36px 6px 16px;
    }

    .ant-tabs-tab-remove {
      position: absolute;
      right: 16px;
    }
  }

  .ant-tabs-content-holder {
    position: relative;
    &::before {
      content: '';
      position: absolute;
      z-index: 10;
      width: 0;
      height: 0;
      display: none;
      transition: all 0.1s ease;
      background: @divider-color;
    }

    .ant-tabs-content.ant-tabs-content-top {
      height: 100%;
    }
  }

  &.drag-over-left {
    .ant-tabs-content-holder {
      &::before {
        width: 50%;
        height: 100%;
        left: 0;
        top: 0;
        display: block;
      }
    }
  }

  &.drag-over-right {
    .ant-tabs-content-holder {
      &::before {
        width: 50%;
        height: 100%;
        left: 50%;
        top: 0;
        display: block;
      }
    }
  }

  &.drag-over-top {
    .ant-tabs-content-holder {
      &::before {
        width: 100%;
        height: 50%;
        left: 0;
        top: 0;
        display: block;
      }
    }
  }

  &.drag-over-bottom {
    .ant-tabs-content-holder {
      &::before {
        width: 100%;
        height: 50%;
        left: 0;
        top: 50%;
        display: block;
      }
    }
  }

  &.drag-over {
    .ant-tabs-content-holder {
      &::before {
        width: 100%;
        height: 100%;
        left: 0;
        top: 0;
        display: block;
      }
    }
  }

  &.is-dragging {
    .ant-tabs-content-holder {
      * {
        pointer-events: none;
      }
    }
  }
}

.ant-tabs-dropdown-menu-item {
  min-width: 80px;

  .ant-tabs-dropdown-menu-title-content {
    display: flex;
    justify-content: space-between;
  }
}
</style>
