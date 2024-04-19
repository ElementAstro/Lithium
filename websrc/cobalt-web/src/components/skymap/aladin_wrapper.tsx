import * as React from "react";

interface aladin_lite_props {
  fov_points: Array<
    [[number, number], [number, number], [number, number], [number, number]]
  >;
  onCenterChange: (new_ra: number, new_dec: number) => void;
  fov_size: number;
  ra: number;
  dec: number;
}

const AladinLiteView: React.FC<aladin_lite_props> = (props) => {
  const alaRef = React.useRef<HTMLDivElement>(null);
  let aladin = React.useRef<any>(null);
  React.useEffect(() => {
    aladin.current = A.aladin("#aladin-lite-div", {
      survey: "P/DSS2/color",
      fov: 60,
      showCooGrid: false,
      showCooGridControl: false,
      projection: "SIN",
      showProjectionControl: false,
      showZoomControl: false,
      showFullscreenControl: false,
      showLayersControl: false,
      showGotoControl: false,
      showFrame: false, // false in development,
      cooframe: "equatorial",
      showSimbadPointrerControl: false,
    });
    aladin.current.setFov(props.fov_size);
    aladin.current.on("positionChanged", function (new_pos: any) {
      props.onCenterChange(new_pos.ra, new_pos.dec);
      // console.log('aladin position changed', new_pos);
    });
  }, []);
  React.useEffect(() => {
    // draw rects
    // console.log('aladin got', props.fov_points);
    aladin.current.removeLayers();
    if (props.fov_points.length > 0) {
      for (let i = 0; i < props.fov_points.length; i++) {
        let current_points_list = props.fov_points[i];
        current_points_list.push(current_points_list[0]); // make a circle
        // console.log('aladin points', current_points_list);
        let overlay = A.graphicOverlay({ color: "#FFFFFF", lineWidth: 2 });
        aladin.current.addOverlay(overlay);
        // need to replace data
        overlay.add(A.polyline(current_points_list));
      }
    }
  }, [props.fov_points]);

  React.useEffect(() => {
    if (aladin.current !== undefined) {
      aladin.current.gotoRaDec(props.ra, props.dec);
    }
  }, [props.ra, props.dec]);

  React.useEffect(() => {
    if (aladin.current !== undefined) {
      aladin.current.setFov(props.fov_size);
    }
  }, [props.fov_size]);

  // event listener for center change is positionChanged
  // another method is always return a ra dec value to its parents when asked?

  return <div id="aladin-lite-div" ref={alaRef}></div>;
};

export default AladinLiteView;
