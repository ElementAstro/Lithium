const cacheName = "my-app-cache";
const cacheFiles = [
  "/",
  "/index.html",
  "/static/js/bundle.js",
  "/static/css/main.css",
  // 添加其他需要缓存的静态资源路径
];

self.addEventListener("install", (event) => {
  event.waitUntil(
    caches.open(cacheName).then((cache) => {
      return cache.addAll(cacheFiles);
    })
  );
});

self.addEventListener("fetch", (event) => {
  event.respondWith(
    caches.match(event.request).then((response) => {
      return response || fetch(event.request);
    })
  );
});
