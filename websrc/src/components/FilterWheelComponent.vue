<template>
    <div>
      <p>Filter wheel data: {{ data.filterWheel }}</p>
      <p>Current filter: {{ data.currentFilter }}</p>
      <p>Number of filters: {{ data.numFilters }}</p>
      <p>Filter positions: {{ data.filterPositions }}</p>
    </div>
  </template>
  
  <script>
  export default {
    props: {
      data: {
        type: Object,
        required: true,
      },
    },
    methods: {
      updateState() {
        // 处理新的数据对象，并更新组件状态
        this.$forceUpdate();
      },
    },
  };
  </script>
  