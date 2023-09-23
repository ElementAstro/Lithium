import { defineConfig } from 'astro/config';
import vue from '@astrojs/vue';
import sitemap from '@astrojs/sitemap';
import astroI18next from "astro-i18next";
import compressor from "astro-compressor";

import markdoc from "@astrojs/markdoc";

// https://astro.build/config
export default defineConfig({
    integrations: [vue({
        template: {
            compilerOptions: {
                // 将任意以 ion- 开头的标签当做自定义元素
                isCustomElement: tag => tag.startsWith('ion-')
            }
        }
        // ...
    }), sitemap(), astroI18next(), compressor(), markdoc()]
});