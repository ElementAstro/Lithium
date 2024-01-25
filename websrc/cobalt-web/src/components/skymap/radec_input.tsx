import React from "react";
import { FormControl, InputGroup, Button } from "react-bootstrap";

interface IInputProps {
  value: number;
  onChange: (newValue: number) => void;
}

function degToDMS(deg: number) {
  // Get the degrees, minutes, and seconds
  let degrees = Math.floor(deg);
  let minutes = Math.floor((deg - degrees) * 60);
  let seconds = Math.round((deg - degrees - minutes / 60) * 3600);

  // Return the DMS values as a string
  return { degrees, minutes, seconds };
}

function dmsToDeg(degrees: number, minutes: number, seconds: number) {
  // Calculate the decimal degrees value
  return degrees + minutes / 60 + seconds / 3600;
}

export const RaInput: React.FC<IInputProps> = (props) => {
  const [degree, set_degree] = React.useState("0");
  const [minute, set_minute] = React.useState("0");
  const [second, set_second] = React.useState("0");
  const [degree_value, set_degree_value] = React.useState("0");
  const [degree_update, set_degree_update] = React.useState(false);
  const [dms_update, set_dms_update] = React.useState(false);
  const [degree_switch, set_degree_switch] = React.useState(false);

  React.useEffect(() => {
    // on props value changed, update value
    set_degree_value(String(props.value));
    let new_dms = degToDMS(props.value);
    set_degree(String(new_dms.degrees));
    set_minute(String(new_dms.minutes));
    set_second(String(new_dms.seconds));
  }, [props.value]);

  React.useEffect(() => {
    if (degree_update) {
      let new_dms = degToDMS(parseFloat(degree_value));
      set_degree(String(new_dms.degrees));
      set_minute(String(new_dms.minutes));
      set_second(String(new_dms.seconds));
      set_degree_update(false);
      props.onChange(parseFloat(degree_value));
    }
  }, [degree_update]);
  React.useEffect(() => {
    if (dms_update) {
      let new_degree = dmsToDeg(
        parseInt(degree),
        parseInt(minute),
        parseFloat(second)
      );
      set_degree_value(String(new_degree));
      set_dms_update(false);
      props.onChange(new_degree);
    }
  }, [dms_update]);

  const on_blur_update_value = () => {
    if (degree_switch) {
      set_degree_update(true);
    } else {
      set_dms_update(true);
    }
  };

  return (
    <>
      <InputGroup>
        {degree_switch ? (
          <InputGroup.Text>
            <Button
              variant="link"
              size="sm"
              onClick={() => {
                set_degree_switch(!degree_switch);
              }}
            >
              RA
            </Button>
          </InputGroup.Text>
        ) : (
          <InputGroup.Text>
            <Button
              variant="link"
              size="sm"
              onClick={() => {
                set_degree_switch(!degree_switch);
              }}
            >
              RA
            </Button>
          </InputGroup.Text>
        )}
        {degree_switch ? (
          <FormControl
            onBlur={() => {
              on_blur_update_value();
            }}
            type="number"
            value={degree_value}
            onChange={(event) => {
              set_degree_value(event.target.value);
              let new_value = event.target.value;
              if (parseInt(new_value) > 180) {
                set_degree_value("180");
              } else if (parseInt(new_value) < -180) {
                set_degree_value("-180");
              }
            }}
            aria-label="RA"
          />
        ) : (
          <>
            <FormControl
              onBlur={() => {
                on_blur_update_value();
              }}
              type="number"
              value={degree}
              onChange={(event) => {
                set_degree(event.target.value);
                let new_value = event.target.value;
                if (parseInt(new_value) > 180) {
                  set_degree("180");
                } else if (parseInt(new_value) < -180) {
                  set_degree("-180");
                }
              }}
              aria-label="RA"
            />
            <FormControl
              onBlur={() => {
                on_blur_update_value();
              }}
              type="number"
              value={minute}
              onChange={(event) => {
                set_minute(event.target.value);
                let new_value = event.target.value;
                if (parseInt(new_value) >= 60) {
                  set_minute("59");
                } else if (parseInt(new_value) < 0) {
                  set_minute("0");
                }
                if (parseInt(degree) === 180 || parseInt(degree) === -180) {
                  set_minute("0");
                }
              }}
              aria-label="RA minutes"
            />
            <FormControl
              onBlur={() => {
                on_blur_update_value();
              }}
              type="number"
              value={second}
              onChange={(event) => {
                set_second(event.target.value);
                let new_value = event.target.value;
                if (parseInt(new_value) >= 60) {
                  set_second("59");
                } else if (parseInt(new_value) < 0) {
                  set_second("0");
                }
                if (parseInt(degree) === 180 || parseInt(degree) === -180) {
                  set_second("0");
                }
              }}
              aria-label="RA seconds"
            />
          </>
        )}
      </InputGroup>
    </>
  );
};

export const DecInput: React.FC<IInputProps> = (props) => {
  const [degree, set_degree] = React.useState("0");
  const [minute, set_minute] = React.useState("0");
  const [second, set_second] = React.useState("0");
  const [degree_value, set_degree_value] = React.useState("0");
  const [degree_update, set_degree_update] = React.useState(false);
  const [dms_update, set_dms_update] = React.useState(false);
  const [degree_switch, set_degree_switch] = React.useState(false);

  React.useEffect(() => {
    // on props value changed, update value
    set_degree_value(String(props.value));
    let new_dms = degToDMS(props.value);
    set_degree(String(new_dms.degrees));
    set_minute(String(new_dms.minutes));
    set_second(String(new_dms.seconds));
  }, [props.value]);

  React.useEffect(() => {
    if (degree_update) {
      let new_dms = degToDMS(parseFloat(degree_value));
      set_degree(String(new_dms.degrees));
      set_minute(String(new_dms.minutes));
      set_second(String(new_dms.seconds));
      set_degree_update(false);
      props.onChange(parseFloat(degree_value));
    }
  }, [degree_update]);
  React.useEffect(() => {
    if (dms_update) {
      let new_degree = dmsToDeg(
        parseInt(degree),
        parseInt(minute),
        parseFloat(second)
      );
      set_degree_value(String(new_degree));
      set_dms_update(false);
      props.onChange(new_degree);
    }
  }, [dms_update]);

  const on_blur_update_value = () => {
    if (degree_switch) {
      set_degree_update(true);
    } else {
      set_dms_update(true);
    }
  };

  return (
    <>
      <InputGroup>
        {degree_switch ? (
          <InputGroup.Text>
            <Button
              variant="link"
              size="sm"
              onClick={() => {
                set_degree_switch(!degree_switch);
              }}
            >
              DEC
            </Button>
          </InputGroup.Text>
        ) : (
          <InputGroup.Text>
            <Button
              variant="link"
              size="sm"
              onClick={() => {
                set_degree_switch(!degree_switch);
              }}
            >
              DEC
            </Button>
          </InputGroup.Text>
        )}
        {degree_switch ? (
          <FormControl
            onBlur={() => {
              on_blur_update_value();
            }}
            type="number"
            value={degree_value}
            onChange={(event) => {
              set_degree_value(event.target.value);
              let new_value = event.target.value;
              if (parseInt(new_value) > 90) {
                set_degree_value("90");
              } else if (parseInt(new_value) < -90) {
                set_degree_value("-90");
              }
            }}
            aria-label="DEC"
          />
        ) : (
          <>
            <FormControl
              onBlur={() => {
                on_blur_update_value();
              }}
              type="number"
              value={degree}
              onChange={(event) => {
                set_degree(event.target.value);
                let new_value = event.target.value;
                if (parseInt(new_value) > 90) {
                  set_degree("90");
                } else if (parseInt(new_value) < -90) {
                  set_degree("-90");
                }
              }}
              aria-label="DEC"
            />
            <FormControl
              onBlur={() => {
                on_blur_update_value();
              }}
              type="number"
              value={minute}
              onChange={(event) => {
                set_minute(event.target.value);
                let new_value = event.target.value;
                if (parseInt(new_value) >= 60) {
                  set_minute("59");
                } else if (parseInt(new_value) < 0) {
                  set_minute("0");
                }
                if (parseInt(degree) === 90 || parseInt(degree) === -90) {
                  set_minute("0");
                }
              }}
              aria-label="DEC minutes"
            />
            <FormControl
              onBlur={() => {
                on_blur_update_value();
              }}
              type="number"
              value={second}
              onChange={(event) => {
                set_second(event.target.value);
                let new_value = event.target.value;
                if (parseInt(new_value) >= 60) {
                  set_second("59");
                } else if (parseInt(new_value) < 0) {
                  set_second("0");
                }
                if (parseInt(degree) === 90 || parseInt(degree) === -90) {
                  set_second("0");
                }
              }}
              aria-label="DEC seconds"
            />
          </>
        )}
      </InputGroup>
    </>
  );
};
