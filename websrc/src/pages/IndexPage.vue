<template>
  <q-page class="login-page" :style="{ backgroundImage: `url(${bgImage})`, backgroundSize: 'cover' }">
    <q-form @submit.prevent="submitForm" class="form-container">
      <div class="form-row q-mb-md q-mt-none">
        <q-input outlined dense v-model="form.host" label="Host" hint="Enter host address, default is 127.0.0.1"
          prepend-icon="mdi-server-network" class="q-mr-md" />
        <q-input outlined dense v-model.number="form.port" label="Port" type="number"
          hint="Enter port number, default is 5000" prepend-icon="mdi-numeric" class="q-mr-md" />
        <q-toggle v-model="form.encrypt" color="primary" label="Encrypt" left-label class="q-mt-sm" />
      </div>
      <div class="form-row">
        <q-input outlined dense v-model="form.username" label="Username" hint="Enter your username"
          prepend-icon="mdi-account" class="q-mr-md" :rules="[val => !!val || 'Please enter a username']" />
        <q-input outlined dense v-model="form.password" label="Password" hint="Optional" type="password"
          prepend-icon="mdi-lock" />
      </div>
      <div class="form-row">
        <q-btn icon="fa-solid fa-image" round size="md" color="primary"  @click="showFileChooser" class="q-mr-md"/>
        <input ref="fileInput" type="file" accept="image/*" style="display:none" @change="changeBackgroundImage">
        <q-btn type="submit" color="primary" label="Log In" :loading="isLoading" :disabled="isLoading"
          class="q-mt-sm full-width" />
      </div> 
    </q-form>
    <q-dialog v-model="errorDialog" title="Error">
      <div class="close-button">
        <q-btn round dense icon="mdi-close" @click="errorDialog = false"></q-btn>
      </div>
      <q-card>
        <q-card-section>
          <div>
            <p>Oops! Something went wrong:</p>
            <pre>{{ error }}</pre>
            <p>Please try again later or contact support if the problem persists.</p>
          </div>
        </q-card-section>
        <q-card-section align="right">
          <q-btn label="Email" icon="fa-solid fa-envelope" color="primary" class="q-mt-sm full-width" @click="openEmail" />
          <q-btn label="OK" icon="fa-solid fa-check" color="primary" class="q-mt-sm full-width" @click="errorDialog = false" />
        </q-card-section>
      </q-card>
    </q-dialog>
  </q-page>
</template>

<script>
export default {
  name: 'LoginPage',
  data() {
    return {
      form: {
        host: '127.0.0.1',
        port: 5000,
        encrypt: false,
        username: '',
        password: ''
      },
      isLoading: false,
      bgImage: require('../assets/background.png'),
      errorDialog: false,
      error: ''
    };
  },
  mounted() {
    if (window.screen.orientation.type.includes('landscape')) {
      document.body.style.overflow = 'hidden';
    }
  },
  methods: {
    submitForm() {
      console.log(this.form);
      this.isLoading = true;
      if (!this.isHostValid()) {
        this.isLoading = false;
        this.error = 'Invalid host address';
        this.errorDialog = true;
        return;
      }
      fetch('/api/login', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json'
        },
        body: JSON.stringify(this.form)
      })
        .then(response => {
          if (!response.ok) {
            throw new Error(response.statusText);
          }
          return response.json();
        })
        .then(result => {
          console.log(result);
          this.isLoading = false;
          this.$router.push('/device');
        })
        .catch(error => {
          console.error(error);
          this.isLoading = false;
          this.error = error.message || 'Login failed';
          this.errorDialog = true;
        });
    },
    isHostValid() {
      const regex = new RegExp(/^(?:[0-9]{1,3}\.){3}[0-9]{1,3}$/);
      return regex.test(this.form.host);
    },
    showFileChooser() {
      this.$refs.fileInput.click();
    },
    changeBackgroundImage(event) {
      const file = event.target.files[0];
      if (!file || !file.type.includes('image/')) {
        return;
      }
      const reader = new FileReader();
      reader.readAsDataURL(file);
      reader.onload = () => {
        this.bgImage = reader.result;
      };
    },
    openEmail() {
      const emailUrl = `mailto:support@example.com?subject=Login+error&body=${encodeURIComponent(this.error.stack)}`;
      window.open(emailUrl, '_blank');
    }
  }
};
</script>

<style scoped>
.login-page {
  height: 100vh;
  display: flex;
  flex-direction: column;
  justify-content: center;
  align-items: center;
}

.logo-container {
  display: flex;
  flex-direction: row;
  justify-content: center;
  align-items: center;
  margin-bottom: 20px;
}

.form-container {
  background-color: rgba(255, 255, 255, 0.8);
  padding: 20px;
  border-radius: 10px;
  margin-bottom: 20px;
}

.form-row {
  display: flex;
  flex-direction: row;
  justify-content: center;
  align-items: center;
  margin-bottom: 20px;
}

.q-input__label,
.q-toggle__label {
  font-size: 14px !important;
}

.q-input__hints {
  font-size: 12px !important;
}

.error-dialog {
  .q-card {
    max-width: 400px;
    color: white;
    background-color: deepskyblue;

    .q-card-section {
      font-size: 14px;
      padding: 20px;

      &:first-child {
        border-bottom: 1px solid #fff;
      }
    }

    .q-btn {
      margin: 0 20px 20px 0;
    }
    .close-button {
      display: flex;
      justify-content: flex-end;
    }
  }
}
</style>