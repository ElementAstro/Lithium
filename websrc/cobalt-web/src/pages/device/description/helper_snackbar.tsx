import * as React from "react";
import { Toast, ToastContainer } from "react-bootstrap";
import { XCircle } from "react-bootstrap-icons";

interface HelperSnackbarProps {
  help_text: string;
  close_signal: boolean;
}

const HelperSnackbar: React.ForwardRefRenderFunction<
  HelperHandle,
  HelperSnackbarProps
> = (props, ref) => {
  const [open_snackbar, setOpen_snackbar] = React.useState(false);

  const handleSnackbarClose = () => {
    setOpen_snackbar(false);
  };

  React.useImperativeHandle(ref, () => ({
    open_snackbar() {
      setOpen_snackbar(true);
    },
  }));

  React.useEffect(() => {
    if (props.close_signal) {
      setOpen_snackbar(false);
    }
  }, [props.close_signal]);

  return (
    <ToastContainer position="top-center">
      <Toast
        show={open_snackbar}
        onClose={handleSnackbarClose}
        delay={6000}
        autohide
      >
        <Toast.Body>
          {props.help_text}
          <XCircle className="float-end" onClick={handleSnackbarClose} />
        </Toast.Body>
      </Toast>
    </ToastContainer>
  );
};

export default React.forwardRef(HelperSnackbar);
