import { defineConfig } from 'astro/config';
import sitemap from '@astrojs/sitemap';
import astroI18next from "astro-i18next";
import compressor from "astro-compressor";
import markdoc from "@astrojs/markdoc";
import react from "@astrojs/react";

// https://astro.build/config
export default defineConfig({
  integrations: [sitemap(), astroI18next(), compressor(), markdoc(), react()],
});