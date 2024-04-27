import * as React from "react";
import { Button, ButtonProps } from "react-bootstrap";

interface SquareButtonProps extends ButtonProps {
  children: React.ReactNode;
}

const SquareButton: React.FC<SquareButtonProps> = (props) => {
  const { children, ...other } = props;
  return (
    <div
      style={{
        position: "relative",
        width: "100%",
        paddingTop: "100%",
      }}
    >
      <Button
        {...other}
        style={{
          position: "absolute",
          width: "100%",
          height: "100%",
          top: 0,
          left: 0,
        }}
      >
        {children}
      </Button>
    </div>
  );
};

export default SquareButton;
