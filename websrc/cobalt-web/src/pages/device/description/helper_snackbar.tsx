import React from 'react';
import { useTranslation } from 'react-i18next';
import { Container, Row, Col, Button, Toast } from 'react-bootstrap';
import { Link, Line } from 'react-bootstrap-icons';

interface HelperSnackbarProps {
  help_text: string;
  close_signal: boolean;
}

const HelperSnackbar = React.forwardRef(function HelperSnackbar(props: HelperSnackbarProps, ref) {
  const { t } = useTranslation();
  const [show, setShow] = React.useState(false);

  const handleClose = () => setShow(false);

  React.useImperativeHandle(ref, () => ({
    openSnackbar() {
      setShow(true);
    }
  }));

  React.useEffect(() => {
    if (props.close_signal) {
      setShow(false);
    }
  }, [props.close_signal]);

  return (
    <Toast onClose={handleClose} show={show} delay={6000} autohide>
      <Toast.Header>
        <strong className="me-auto">{t('title')}</strong>
      </Toast.Header>
      <Toast.Body>{props.help_text}</Toast.Body>
    </Toast>
  );
});

export default HelperSnackbar;
