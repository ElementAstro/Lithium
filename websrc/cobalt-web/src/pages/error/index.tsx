import React, { useEffect, useState } from "react";
import { useSearchParams } from "react-router-dom";
import {
  Body,
  Container,
  H1,
  H2,
  H4,
  H5,
  Page,
  Details,
  Image,
  StopCode,
  Link,
  QR,
} from "./style";

const ErrorPage = () => {
  const [percentage, setPercentage] = useState(0);
  const [error, setError] = useState(404);
  const [message, setMessage] = useState("Something went wrong");
  const [searchParams, setSearchParams] = useSearchParams();

  const processInterval = () => {
    const intervalId = setInterval(() => {
      setPercentage((percentage) => {
        const newPercentage = percentage + Math.random() * 10;
        if (newPercentage >= 100) {
          clearInterval(intervalId);
          return 100;
        }
        return newPercentage;
      });
    }, Math.random() * (1000 - 500) + 500);
  };

  const process = () => {
    setPercentage(0);
    processInterval();
  };

  useEffect(() => {
    if (searchParams.get("error") && searchParams.get("message")) {
      setError(parseInt(searchParams.get("error") as string));
      setMessage(searchParams.get("message") as string);
    }
    processInterval();
  }, []);

  return (
    <Body>
      <Page>
        <Container>
          <H1>:(</H1>
          <H2>
            Your PC ran into a problem and needs to restart. We're just
            collecting some error info, and then we'll restart for you.
          </H2>
          <H2>
            <span id="percentage">{percentage}</span>% complete
          </H2>
          <Details>
            <QR>
              <Image></Image>
            </QR>
            <StopCode>
              <H4>
                For more information about this issue and possible fixes, visit
                <br />
                <Link href="https://github.com/ElementAstro/Lithium/issues">
                  https://github.com/ElementAstro/Lithium/issues
                </Link>
              </H4>
              <H5>
                If you call a support person, give them this info: <br />
                Stop Code: "{error}"<br />
                Error Message: "{message}"
              </H5>
            </StopCode>
          </Details>
        </Container>
      </Page>
    </Body>
  );
};

export default ErrorPage;
