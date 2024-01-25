// CameraView.jsx
import React, { useState, useEffect, useRef } from "react";
import Cropper, { ReactCropperElement } from "react-cropper";
import "cropperjs/dist/cropper.css";
import axios from "axios";

const CameraView = () => {
  const [imageData, setImageData] = useState("");
  const [refreshKey, setRefreshKey] = useState(0);
  const cropperRef = useRef<ReactCropperElement>(null);
  const onCrop = () => {
    const cropper = cropperRef.current?.cropper;
    console.log(cropper.getCroppedCanvas().toDataURL());
  };

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

  return (
    <div className="camera-view">
      <button onClick={handleRefresh}>刷新</button>
    </div>
  );
};

export default CameraView;
