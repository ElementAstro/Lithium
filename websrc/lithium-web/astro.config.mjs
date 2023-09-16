import { defineConfig } from 'astro/config';
import vue from '@astrojs/vue';

// https://astro.build/config
export default defineConfig({
    integrations: [
        vue({
            template: {
                compilerOptions: {
                    // 将任意以 ion- 开头的标签当做自定义元素
                    isCustomElement: (tag) => tag.startsWith('ion-'),
                },
            },
            // ...
        }),
    ],
});
