<template>
    <div class="q-pa-md">
        <div class="viewer-container">
            <div class="image-container">
                <img v-show="parsedImage" :src="parsedImage" alt="Parsed Image" />
            </div>
        </div>

        <div class="q-mt-md">
            <div class="q-gutter-md">
                <q-input label="Right Ascension" type="number" v-model="ra" :min="-360" :max="360" step="0.1" />
                <q-input label="Declination" type="number" v-model="dec" :min="-90" :max="90" step="0.1" />
                <q-input label="FOV" type="number" v-model="fov" :min="0" :max="180" step="0.1" />
                <q-input label="Range" type="number" v-model="range" :min="0" step="0.1" />
            </div>

            <q-upload class="q-mt-md" @uploaded="onFileUploaded" :accept="['.jpg', '.png', '.jpeg']" :multiple="false"
                :max-size="5000000" :capture="true">
                {{ uploadedFileName || 'Upload Image' }}
            </q-upload>
        </div>
    </div>

    <div>
    </div>
</template>
  
<style scoped>
.image-container {
    position: relative;
}

.image-container img {
    width: 100%;
    height: auto;
    display: block;
}
</style>
  
<script>
import axios from 'axios';

export default {
    props: {
        parsedData: {
            type: Object,
            required: true,
        },
    },
    data() {
        return {
            ra: 0,
            dec: 0,
            fov: 180,
            range: 0,
            uploadedFile: null,
            uploadedFileName: null,
            parsedImage: null,
            isLoading: false,
        };
    },
    methods: {
        async onFileUploaded(files) {
            this.uploadedFile = files[0];
            this.uploadedFileName = this.uploadedFile.name;
            const formData = new FormData();
            formData.append('image', this.uploadedFile);

            this.isLoading = true;

            try {
                const response = await axios.post('/api/parse-image', formData);
                this.parsedImage = response.data.url;
            } catch (error) {
                console.error(error);
            }

            this.isLoading = false;
        },
    },
};
</script>
  