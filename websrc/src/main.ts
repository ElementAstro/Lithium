import './assets/main.css'

import { createApp } from 'vue'
import { createPinia } from 'pinia'

import App from './App.vue'
import router from './router'

import { create, NMessageProvider } from 'naive-ui'

const naive = create({
  components: [NMessageProvider]
})

const app = createApp(App)

app.use(createPinia())
app.use(naive)
app.use(router)

app.mount('#app')
