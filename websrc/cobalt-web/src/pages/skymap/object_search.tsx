import React, { useState } from "react";
import { Form, Alert, Card } from "react-bootstrap";
import { FilesAlt, ChevronDown, ChevronUp } from "react-bootstrap-icons";
import styled from "styled-components";

import TargetSmallCardHori from "../../components/skymap/target_smallv2";
import TargetSmallCard from "../../components/skymap/target_small";

import * as AXIOSOF from "../../services/object_finding_api";

interface ObjectSearchProps {
  on_choice_maken: (() => void) | null;
}

const SearchContainer = styled.div`
  display: flex;
  flex-direction: row;
`;

const SearchInputContainer = styled.div`
  flex: 1;
  margin-right: 10px;
`;

const FilterContainer = styled(Card)`
  flex: 1;
`;

const FilterHeader = styled(Card.Header)`
  cursor: pointer;
`;

const ObjectSearch: React.FC<ObjectSearchProps> = (props) => {
  // ui control
  const [expand_filter, set_expand_filter] = useState(false);
  const [alert_type, set_alert_type] = useState("info");
  const [alert_text, set_alert_text] =
    useState("请在左侧输入框输入需要搜索的信息");
  // data
  const [to_search_text, set_to_search_text] = useState("-");
  const [found_target_result, set_found_target_result] = useState<
    Array<IDSOObjectDetailedInfo>
  >([]);
  const [filter_settings, update_filter_settings] = useState({
    angular_size: 1, // in arc degree
  });

  const handleFilterExpandClick = () => {
    set_expand_filter(!expand_filter);
  };
  const handleSearchTextOnBlur = async () => {
    if (to_search_text === "-") {
      return null;
    }
    set_found_target_result([]);
    set_alert_type("info");
    set_alert_text("查询中");
    try {
      const found_targets = await AXIOSOF.findTargetByName(to_search_text);
      if (found_targets.success) {
        if (found_targets.data.length > 0) {
          set_found_target_result(found_targets.data);
        } else {
          set_alert_type("warning");
          set_alert_text("没有找到相关目标");
        }
      } else {
        set_alert_type("error");
        set_alert_text("没有找到相关目标!!!");
      }
    } catch (err) {
      return null;
    }
  };

  return (
    <SearchContainer>
      <SearchInputContainer>
        {/* search and filter part */}
        <Form.Group onBlur={handleSearchTextOnBlur}>
          <Form.Control
            required
            type="text"
            placeholder="输入搜索关键词"
            value={to_search_text}
            onChange={(event) => {
              set_to_search_text(event.target.value);
            }}
            className="mt-1"
          />
        </Form.Group>
      </SearchInputContainer>
      <FilterContainer>
        <FilterHeader onClick={handleFilterExpandClick}>
          <FilesAlt className="me-2" />
          目标搜索筛选
          {expand_filter ? <ChevronUp /> : <ChevronDown />}
        </FilterHeader>
        <Card.Body>{expand_filter && <h6>开发中</h6>}</Card.Body>
      </FilterContainer>
      <div style={{ flex: 3 }}>
        {/* display cards part */}
        {found_target_result.length === 0 ? (
          <Alert variant={alert_type}>{alert_text}</Alert>
        ) : (
          <div className="row">
            {found_target_result.map((one_dso_target_info, index) => {
              return (
                <div className="col-md-12" key={index}>
                  <TargetSmallCard
                    target_info={one_dso_target_info}
                    on_card_clicked={null}
                    card_index={index}
                    on_choice_maken={props.on_choice_maken}
                    in_manage={false}
                  />
                </div>
              );
            })}
          </div>
        )}
      </div>
    </SearchContainer>
  );
};

ObjectSearch.defaultProps = {
  on_choice_maken: null,
};

export default ObjectSearch;
