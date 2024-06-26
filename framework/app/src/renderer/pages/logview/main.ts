import './setEnv';
import { createApp } from 'vue';
import App from '@kungfu-trader/kungfu-app/src/renderer/pages/logview/App.vue';

import VueVirtualScroller from 'vue-virtual-scroller';
import 'vue-virtual-scroller/dist/vue-virtual-scroller.css';
import { Button, Checkbox, Input, Layout, Spin } from 'ant-design-vue';
import VueI18n from '@kungfu-trader/kungfu-js-api/language';
import { loadCustomFont } from '@kungfu-trader/kungfu-app/src/renderer/assets/methods/uiUtils';

const app = createApp(App);

app
  .use(Layout)
  .use(Button)
  .use(Checkbox)
  .use(Input)
  .use(Spin)
  .use(VueVirtualScroller);

app.use(VueI18n);

loadCustomFont().then(() => app.mount('#app'));
