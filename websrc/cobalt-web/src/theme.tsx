import React from "react";
import { ThemeProvider } from "styled-components";
import { createGlobalStyle } from "styled-components";
import { Container } from "react-bootstrap";

const darkTheme = {
  palette: {
    mode: "dark",
    primary: {
      main: "#3f51b5",
    },
    secondary: {
      main: "#f50057",
    },
    success: {
      main: "#388e3c",
    },
  },
};

const GlobalStyle = createGlobalStyle`
  body {
    background-color: ${({ theme }) =>
      theme.palette.mode === "dark" ? "#121212" : "#fff"};
    color: ${({ theme }) => (theme.palette.mode === "dark" ? "#fff" : "#000")};
  }

  .btn {
    border-radius: 8px;
  }

  .btn-link {
    border-radius: 8px;
  }

  .form-control {
    background-color: rgba(255, 255, 255, 0.5);
  }

  .form-control:focus {
    background-color: rgba(255, 255, 255, 0.5);
  }
`;

export const CustomThemeProvider: React.FC<any> = ({ children }) => {
  return (
    <ThemeProvider theme={darkTheme}>
      <GlobalStyle />
      <Container>{children}</Container>
    </ThemeProvider>
  );
};
