import React from "react";
import ReactDOM from "react-dom/client";

import i18n from "i18next";
import { initReactI18next } from "react-i18next";

import Route from "./routes/root.tsx";
import { Router } from "react-router";

import translationEN from "./locales/en/translation.json";
import translationZH from "./locales/zh/translation.json";

import "./index.less";

i18n
  .use(initReactI18next) // passes i18n down to react-i18next
  .init({
    // the translations
    // (tip move them in a JSON file and import them,
    // or even better, manage them via a UI: https://react.i18next.com/guides/multiple-translation-files#manage-your-translations-with-a-management-gui)
    resources: {
      en: {
        translation: translationEN,
      },
      zh: {
        translation: translationZH,
      },
    },
    lng: "en", // if you're using a language detector, do not define the lng option
    fallbackLng: "en",

    interpolation: {
      escapeValue: false, // react already safes from xss => https://www.i18next.com/translation-function/interpolation#unescape
    },
  });

import "./index.less";
import App from "./App";

ReactDOM.createRoot(document.getElementById("root") as HTMLElement).render(
  <React.StrictMode>
    <App />
  </React.StrictMode>
);
