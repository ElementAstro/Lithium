import { defineConfig } from 'astro/config';
import sitemap from '@astrojs/sitemap';
import compressor from "astro-compressor";
import markdoc from "@astrojs/markdoc";

import node from "@astrojs/node";

// https://astro.build/config
export default defineConfig({
  integrations: [sitemap(), compressor(), markdoc()],
  i18n: {
    defaultLocale: "zh",
    locales: ["es", "en", "zh"],
    routing: {
      prefixDefaultLocale: false
    }
  },
  output: "server",
  adapter: node({
    mode: "standalone"
  })
});