<template>
    <div class="q-pa-md">
      <div class="row full-width q-pb-sm">
        <div class="col">
          <q-input dense outlined clearable filled v-model="searchTerm" label="Insert Object Name" class="full-width"
            size="sm"></q-input>
        </div>
        <div class="col-auto">
          <q-btn push icon="search" color="primary" @click="filterItems">Search</q-btn>
        </div>
        <div class="col-auto">
          <q-btn-toggle v-model="showFilter" color="secondary" :options="[{
              icon: 'filter',
              label: 'Filter',
          }]" />
        </div>
      </div>
  
      <q-separator />
  
      <q-scroll-area style="height: 1000px">
        <q-card v-for="(item, index) in filteredItems" :key="index" @click="copyData(item)">
          <q-item>
            <q-item-section class="q-pr-md">
              <div :style="{ backgroundImage: 'url(' + item.img + ')' }" class="item-image"></div>
            </q-item-section>
            <q-item-section>
              <q-item-label>{{ item.name }}</q-item-label>
              <q-item-label caption>Type: {{ item.type }}</q-item-label>
              <q-item-label>RA: {{ item.ra }}</q-item-label>
              <q-item-label>Dec: {{ item.dec }}</q-item-label>
              <q-item-label>Mag:
                <span :class="magClass(item.mag)">{{ item.mag }}</span>
              </q-item-label>
              <q-item-label>Distance: {{ item.distance }}</q-item-label>
              <q-item-label>Apparent size: {{ item.size }}</q-item-label>
            </q-item-section>
          </q-item>
        </q-card>
      </q-scroll-area>
  
      <q-dialog v-model="showFilter" class="q-pt-xl">
        <q-card>
          <template v-slot:header>Filter Items</template>
          <q-card-section class="q-pa-md">
            <q-input outlined v-model="minMag" type="number" label="Min Magnitude"></q-input>
            <q-input outlined v-model="maxMag" type="number" label="Max Magnitude"></q-input>
            <q-input outlined v-model="minDist" type="number" label="Min Distance"></q-input>
            <q-input outlined v-model="maxDist" type="number" label="Max Distance"></q-input>
          </q-card-section>
          <q-card-actions align="right">
            <q-btn label="Cancel" color="negative" @click="showFilter = false"></q-btn>
            <q-btn label="Apply" color="primary" @click="setFilter"></q-btn>
          </q-card-actions>
        </q-card>
      </q-dialog>
    </div>
  </template>
  
<script>
export default {
    data() {
        return {
            searchTerm: "",
            showFilter: false,
            minMag: null,
            maxMag: null,
            minDist: null,
            maxDist: null,
            itemList: [
                {
                    name: "M31",
                    type: "Galaxy",
                    ra: "00h 42m 44s",
                    dec: "+41° 16' 09\"",
                    mag: "3.4",
                    distance: "2.5 million light years",
                    size: "178,000 light years",
                    img: "https://www.nasa.gov/sites/default/files/thumbnails/image/m31_0.png",
                },
                {
                    name: "Jupiter",
                    type: "Planet",
                    ra: "19h 22m 32s",
                    dec: "-22° 02' 04\"",
                    mag: "-1.66",
                    distance: "778.5 million km",
                    size: "142,984 km",
                    img: "https://www.nasa.gov/sites/default/files/thumbnails/image/hs-2017-07-a-hires_jpg.jpg",
                },
                {
                    name: "Orion Nebula",
                    type: "Nebula",
                    ra: "05h 35m 17s",
                    dec: "-05° 23' 28\"",
                    mag: "4.0",
                    distance: "1,344 light years",
                    size: "24 light years",
                    img: "https://www.nasa.gov/sites/default/files/thumbnails/image/m42_hst_big.jpg",
                },
                // ...
            ],
        };
    },

    computed: {
        filteredItems() {
            const { searchTerm, minMag, maxMag, minDist, maxDist } = this;

            return this.itemList.filter(({ name, type, mag, distance }) => {
                const nameMatches = name.toLowerCase().includes(searchTerm);
                const typeMatches = type.toLowerCase().includes(searchTerm);

                const magInRange =
                    (minMag === null || parseFloat(mag) >= parseFloat(minMag)) &&
                    (maxMag === null || parseFloat(mag) <= parseFloat(maxMag));

                const distInRange =
                    (minDist === null || parseFloat(distance) >= parseFloat(minDist)) &&
                    (maxDist === null || parseFloat(distance) <= parseFloat(maxDist));

                return nameMatches || typeMatches || magInRange || distInRange;
            });
        },
    },

    methods: {
        setFilter() {
            this.showFilter = false;
        },

        filterItems() {
            this.$forceUpdate();
        },

        copyData(item) {
            const el = document.createElement("textarea");
            el.value = `
          Name: ${item.name}
          Type: ${item.type}
          RA: ${item.ra}
          Dec: ${item.dec}
          Mag: ${item.mag}
          Distance: ${item.distance}
          Apparent size: ${item.size}
        `;

            document.body.appendChild(el);
            el.select();
            document.execCommand("copy");
            document.body.removeChild(el);

            this.$q.notify({
                message: "Copied to clipboard",
                color: "positive",
            });
        },

        magClass(mag) {
            if (parseFloat(mag) < 0) return "mag-bright";
            if (parseFloat(mag) >= 0 && parseFloat(mag) < 5) return "mag-medium";
            return "mag-faint";
        },
    },
};
</script>
  
<style scoped>
.q-input {
    width: 200px;
}

.q-item-section:first-child div.item-image {
    height: 100px;
    width: 100px;
    background-size: cover;
}

.mag-bright {
    color: #ffdd00;
}

.mag-medium {
    color: #ffffff;
}

.mag-faint {
    color: #a6a6a6;
}
</style>
  