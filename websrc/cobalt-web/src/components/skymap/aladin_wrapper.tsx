import React, { useRef, useEffect, memo } from "react";

const AladinLiteView = memo(
  ({
    fovPoints,
    onCenterChange,
    fovSize,
    ra,
    dec,
  }: {
    fovPoints: Array<
      [[number, number], [number, number], [number, number], [number, number]]
    >;
    onCenterChange: (ra: number, dec: number) => void;
    fovSize: number;
    ra: number;
    dec: number;
  }) => {
    const alaRef = useRef(null);
    let aladin: any = null;

    useEffect(() => {
      aladin = A.aladin("#aladin-lite-div", {
        survey: "P/DSS2/color",
        fov: 60,
        showCooGrid: true,
        showCooGridControl: true,
        projection: "SIN",
        showProjectionControl: false,
        showZoomControl: true,
        showFullscreenControl: false,
        showLayersControl: false,
        showGotoControl: false,
        showFrame: true,
        cooframe: "equatorial",
        showSimbadPointrerControl: false,
      });

      aladin.on(
        "positionChanged",
        ({ ra, dec }: { ra: number; dec: number }) => {
          onCenterChange(ra, dec);
        }
      );

      return () => {
        aladin = null; // 清理 aladin 实例
      };
    }, []);

    useEffect(() => {
      if (aladin && fovPoints.length > 0) {
        aladin.removeLayers();
        const overlay = A.graphicOverlay({ color: "#ee2345", lineWidth: 2 });
        aladin.addOverlay(overlay);
        overlay.add(A.polyline(fovPoints));
      }
    }, [fovPoints]);

    useEffect(() => {
      if (aladin) {
        aladin.gotoRaDec(ra, dec);
      }
    }, [ra, dec]);

    return <div id="aladin-lite-div" ref={alaRef}></div>;
  }
);

export default AladinLiteView;
