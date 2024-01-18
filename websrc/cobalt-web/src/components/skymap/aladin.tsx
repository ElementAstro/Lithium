import React from "react";

const Alading = () => {
  const alaRef = React.useRef<HTMLDivElement>(null);

  React.useEffect(() => {
    A.init.then(() => {
      A.aladin(alaRef.current, {
        fov: 360,
        projection: "AIT",
        cooFrame: "equatorial",
        showCooGridControl: true,
        showSimbadPointerControl: true,
        showCooGrid: true,
      });
    });
  }, []);
  return <div ref={alaRef} style={{ width: 500, height: 400 }}></div>;
};

export default Alading;
