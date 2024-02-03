// CameraView.jsx
import React, { useState, useEffect, useRef } from "react";
import Cropper, { ReactCropperElement } from "react-cropper";
import "cropperjs/dist/cropper.css";
import axios from "axios";
import { Button, Modal } from "react-bootstrap";
import { Crop, ArrowRepeat } from "react-bootstrap-icons";

const CameraView = () => {
  const [imageData, setImageData] = useState("");
  const [refreshKey, setRefreshKey] = useState(0);
  const [croppedData, setCroppedData] = useState("");
  const [showModal, setShowModal] = useState(false);
  const cropperRef = useRef<ReactCropperElement>(null);

  useEffect(() => {
    // 发送请求获取最新的图片
    axios.get("/api/getImage").then((response) => {
      setImageData(response.data);
    });
  }, [refreshKey]);

  const handleRefresh = () => {
    // 更新刷新键以重新加载图片
    setRefreshKey((prevKey) => prevKey + 1);
  };

  const handleCrop = () => {
    const cropper = cropperRef.current?.cropper;
    if (cropper) {
      setCroppedData(cropper.getCroppedCanvas().toDataURL());
      setShowModal(true);
    }
  };

  const handleSave = () => {
    // 将裁剪后的图片数据发送到服务器进行保存
    axios.post("/api/saveImage", { imageData: croppedData }).then(() => {
      setShowModal(false);
      // 刷新页面以更新图片
      handleRefresh();
    });
  };

  return (
    <div className="camera-view">
      <Cropper
        src={imageData}
        ref={cropperRef}
        aspectRatio={16 / 9}
        guides={false}
      />
      <button onClick={handleCrop}>
        <Crop /> 裁剪
      </button>
      <button onClick={handleRefresh}>
        <ArrowRepeat /> 刷新
      </button>

      <Modal
        show={showModal}
        onHide={() => setShowModal(false)}
        backdrop="static"
        keyboard={false}
      >
        <Modal.Header closeButton>
          <Modal.Title>编辑图片</Modal.Title>
        </Modal.Header>
        <Modal.Body>
          <img src={croppedData} alt="Cropped" />
        </Modal.Body>
        <Modal.Footer>
          <Button variant="secondary" onClick={() => setShowModal(false)}>
            取消
          </Button>
          <Button variant="primary" onClick={handleSave}>
            保存
          </Button>
        </Modal.Footer>
      </Modal>
    </div>
  );
};

export default CameraView;
