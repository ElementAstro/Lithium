import React from "react";
import { GlobalStore } from "../../../store/globalStore";
import { Card, Button, Row, Col } from "react-bootstrap";
import { Compass } from "react-bootstrap-icons";

const GPLocationEasyUseComp: React.FC = () => {
  const global_parameter = GlobalStore.useAppState(
    (state) => state.GlobalParameterStore
  );

  const [error_in_range, set_error_in_range] = React.useState(false);

  const calculate_distance = () => {
    if (
      "GlobalLocation" in window &&
      global_parameter.global_parameter.geo_location !== null
    ) {
      let GlobalLocation = (window as any).GlobalLocation as {
        lng: number;
        lat: number;
      };
      const long1 = GlobalLocation.lng as number;
      const lat1 = GlobalLocation.lat as number;
      const long2 = global_parameter.global_parameter.geo_location.longitude;
      const lat2 = global_parameter.global_parameter.geo_location.latitude;
      // calculate distance on earth between two points. unit in km.
      const R = 6371;
      const dLat = ((lat2 - lat1) * Math.PI) / 180;
      const dLon = ((long2 - long1) * Math.PI) / 180;
      const a =
        Math.sin(dLat / 2) * Math.sin(dLat / 2) +
        Math.cos((lat1 * Math.PI) / 180) *
          Math.cos((lat2 * Math.PI) / 180) *
          Math.sin(dLon / 2) *
          Math.sin(dLon / 2);
      const c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
      const d = R * c;
      return d;
    } else {
      return -1;
    }
  };
  const on_import_location_2_gp_setting = () => {
    if ("GlobalLocation" in window) {
      let GlobalLocation = (window as any).GlobalLocation as {
        lng: number;
        lat: number;
      };
      GlobalStore.actions.GlobalParameterStore.set_parameter({
        parameter_name: "geo_location",
        to_set_parameter: {
          longitude: GlobalLocation.lng,
          latitude: GlobalLocation.lat,
          // height:
        },
      });
    }
  };

  React.useEffect(() => {
    const distance = calculate_distance();
    if (distance !== -1 && distance > 5) {
      set_error_in_range(false);
    } else {
      set_error_in_range(true);
    }
  }, []);
  React.useEffect(() => {
    const distance = calculate_distance();
    if (distance !== -1 && distance > 5) {
      set_error_in_range(false);
    } else {
      set_error_in_range(true);
    }
  }, [
    global_parameter.global_parameter.geo_location?.latitude,
    global_parameter.global_parameter.geo_location?.longitude,
  ]);
  React.useEffect(() => {
    const distance = calculate_distance();
    if (distance !== -1 && distance > 5) {
      set_error_in_range(false);
    } else {
      set_error_in_range(true);
    }
  }, [window.GlobalLocation?.lat]);

  return (
    <>
      <Row>
        <Col xs={6}>
          {/* 手机定位信息 */}
          {"GlobalLocation" in window ? (
            <Card bg="primary" text="white" style={{ borderRadius: "5px" }}>
              <Card.Body>
                <Compass size={40} />
                <Card.Text>定位经度： {window.GlobalLocation?.lng} °</Card.Text>
                <Card.Text>定位纬度： {window.GlobalLocation?.lat} °</Card.Text>
                <Card.Text>定位高度： 米</Card.Text>
              </Card.Body>
              <Card.Footer>
                {error_in_range ? (
                  <Button variant="success">在范围内</Button>
                ) : (
                  <Button variant="warning">定位坐标和配置误差较大</Button>
                )}
                <Button
                  disabled={!("GlobalLocation" in window)}
                  onClick={on_import_location_2_gp_setting}
                >
                  导入全局变量
                </Button>
              </Card.Footer>
            </Card>
          ) : (
            <Card>
              <Card.Body>
                <Card.Title>未成功获取手机定位</Card.Title>
              </Card.Body>
            </Card>
          )}
        </Col>
        <Col xs={6}>
          {/* 全局变量内部的信息 */}
          {global_parameter.global_parameter.geo_location === null ? (
            <Card>
              <Card.Body>
                <Card.Title>Failed to get phone location</Card.Title>
              </Card.Body>
            </Card>
          ) : (
            <Card bg="primary" text="white">
              <Card.Body>
                <Compass size={40} />
                <Card.Text>
                  系统设置经度：{" "}
                  {global_parameter.global_parameter.geo_location.longitude} °
                </Card.Text>
                <Card.Text>
                  系统设置纬度：{" "}
                  {global_parameter.global_parameter.geo_location.latitude} °
                </Card.Text>
                <Card.Text>
                  系统设置高度：{" "}
                  {global_parameter.global_parameter.geo_location.height} 米
                </Card.Text>
                <Card.Text>
                  系统设置时区：{" "}
                  {global_parameter.global_parameter.geo_location.time_zone}
                </Card.Text>
              </Card.Body>
              <Card.Footer>
                <Button>手动输入</Button>
              </Card.Footer>
            </Card>
          )}
        </Col>
      </Row>
      <Row>
        <Col xs={12}>
          <Card>
            <Card.Body>
              <Card.Text>
                <strong className="text-warning">
                  Please confirm the longitude and latitude information before
                  starting device connection!
                </strong>
                <br />
                If you modify the system's longitude, latitude or other
                information after connecting to the equatorial mount, it will
                not automatically sync to the equatorial mount. If you have
                modified the location information, you must disconnect and
                reconnect the equatorial mount!
              </Card.Text>
            </Card.Body>
          </Card>
        </Col>
      </Row>
    </>
  );
};

export default GPLocationEasyUseComp;
