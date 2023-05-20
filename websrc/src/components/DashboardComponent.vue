<template>
    <div class="dashboard-component">
        <div class="viewer-container">
            <img :src="data.latestImage" class="viewer-image" />
        </div>
        <div class="actions-container">
            <q-btn flat icon="mdi-cog" @click="openSettings"></q-btn>
            <q-btn flat icon="mdi-camera-iris" @click="takeExposure"></q-btn>
            <q-btn flat icon="mdi-tune" @click="openExposureDialog"></q-btn>
        </div>
        <q-dialog v-model="exposureDialog" title="Adjust Exposure">
            <q-card>
                <q-card-section>
                    <div>
                        <p>Current exposure time: {{ data.exposureTime }}</p>
                        <q-input type="number" v-model.number="newExposureTime" min="0" suffix="s" outlined dense />
                    </div>
                </q-card-section>
                <q-card-actions align="right">
                    <q-btn label="Cancel" icon="fa-solid fa-times" color="primary" class="q-mt-sm"
                        @click="exposureDialog = false" />
                    <q-btn label="OK" icon="fa-solid fa-check" color="primary" class="q-mt-sm" @click="setExposureTime" />
                </q-card-actions>
            </q-card>
        </q-dialog>
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
    data() {
        return {
            exposureDialog: false,
            newExposureTime: 0,
        };
    },
    methods: {
        openSettings() {
            // TODO: Implement settings logic
        },
        takeExposure() {
            // TODO: Implement exposure logic
        },
        openExposureDialog() {
            this.exposureDialog = true;
        },
        setExposureTime() {
            // TODO: Implement exposure time setting logic
            this.exposureDialog = false;
        },
    },
};
</script>

<style scoped>
.dashboard-component {
    display: flex;
    flex-direction: row;
    align-items: center;
}

.viewer-container {
    width: 60%;
    min-width: 400px;
    margin-right: 20px;
}

.viewer-image {
    max-width: 100%;
    border-radius: 4px;
}

.actions-container {
    display: flex;
    flex-direction: column;
}

.q-btn {
    margin-bottom: 10px;
}
</style>
