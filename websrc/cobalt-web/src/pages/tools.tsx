import React, { useState, useEffect } from 'react';
import axios from 'axios';
import { Button, Modal } from 'react-bootstrap';

function ToolsComponent() {
  const [time, setTime] = useState('');
  const [lon, setLon] = useState('');
  const [lat, setLat] = useState('');
  const [errorMessage, setErrorMessage] = useState('');
  const [showModal, setShowModal] = useState(false);
  const [modalMessage, setModalMessage] = useState('');

  function onError(message) {
    setModalMessage(message);
    setShowModal(true);
  }

  function getTime() {
    let now = new Date();
    let year = now.getFullYear();
    let month = now.getMonth() + 1;
    let day = now.getDate();
    let hour = now.getHours();
    let minute = now.getMinutes();
    let second = now.getSeconds();
    let time = `${year}:${month}:${day}-${hour}:${minute}:${second}`;
    console.log(time);
    setTime(time);
    return time;
  }

  function syncTime() {
    axios
      .get(`/tools/api/time/${getTime()}`)
      .then((response) => {
        if (!response.data.error) {
          console.log(response.data.message);
          setModalMessage(`Info: ${response.data.message}`);
          setShowModal(true);
        } else {
          onError(`Error: ${response.data.error}`);
        }
      })
      .catch((error) => {
        onError('Failed to send request to server');
      });
  }

  function resetTime() {
    setTime('');
  }

  function getLocation() {
    let options = {
      enableHighAccuracy: true,
      maximumAge: 1000,
    };
    if (navigator.geolocation) {
      navigator.geolocation.getCurrentPosition(
        onLocationSuccess,
        onLocationError,
        options
      );
    } else {
      onError('Web browser does not support GPS function');
    }
  }

  function syncLocation() {
    axios
      .get(`/tools/api/location/${lon}/${lat}`)
      .then((response) => {
        if (!response.data.error) {
          console.log(response.data.message);
          setModalMessage(`Info: ${response.data.message}`);
          setShowModal(true);
        } else {
          onError(`Error: ${response.data.error}`);
        }
      })
      .catch((error) => {
        onError('Failed to send request to server');
      });
  }

  function resetLocation() {
    setLon('');
    setLat('');
  }

  function onLocationSuccess(pos) {
    console.log(`Lon:${pos.coords.longitude} Lat:${pos.coords.latitude}`);
    setLon(pos.coords.longitude);
    setLat(pos.coords.latitude);
  }

  function onLocationError(error) {
    switch (error.code) {
      case 1:
        onError('Positioning function rejected');
        break;
      case 2:
        onError('Unable to obtain location information temporarily');
        break;
      case 3:
        onError('Get information timeout');
        break;
      case 4:
        onError('Unknown error');
        break;
    }
  }

  useEffect(() => {
    document
      .getElementById('get_time')
      .addEventListener('click', getTime);
    document
      .getElementById('sync_time')
      .addEventListener('click', syncTime);
    document
      .getElementById('reset_time')
      .addEventListener('click', resetTime);
    document
      .getElementById('get_location')
      .addEventListener('click', getLocation);
    document
      .getElementById('sync_location')
      .addEventListener('click', syncLocation);
    document
      .getElementById('reset_location')
      .addEventListener('click', resetLocation);

    return () => {
      document
        .getElementById('get_time')
        .removeEventListener('click', getTime);
      document
        .getElementById('sync_time')
        .removeEventListener('click', syncTime);
      document
        .getElementById('reset_time')
        .removeEventListener('click', resetTime);
      document
        .getElementById('get_location')
        .removeEventListener('click', getLocation);
      document
        .getElementById('sync_location')
        .removeEventListener('click', syncLocation);
      document
        .getElementById('reset_location')
        .removeEventListener('click', resetLocation);
    };
  }, []);

  return (
    <div>
      {/* JSX内容 */}
      <Button id="get_time" onClick={getTime}>
        Get Time
      </Button>
      <Button id="sync_time" onClick={syncTime}>
        Sync Time
      </Button>
      <Button id="reset_time" onClick={resetTime}>
        Reset Time
      </Button>
      <Button id="get_location" onClick={getLocation}>
        Get Location
      </Button>
      <Button id="sync_location" onClick={syncLocation}>
        Sync Location
      </Button>
      <Button id="reset_location" onClick={resetLocation}>
        Reset Location
      </Button>

      <Modal show={showModal} onHide={() => setShowModal(false)}>
        <Modal.Body>{modalMessage}</Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" onClick={() => setShowModal(false)}>
            Close
          </Button>
        </Modal.Footer>
      </Modal>
    </div>
  );
}

export default ToolsComponent;
