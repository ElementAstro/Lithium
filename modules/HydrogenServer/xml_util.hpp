#pragma once

#include <vector>
#include <unordered_map>

#include "lilxml.h"

static int xmlReplacementMapFind(void *self, XMLEle *source, XMLEle **replace);

XMLEle *cloneXMLEleWithReplacementMap(XMLEle *root, const std::unordered_map<XMLEle *, XMLEle *> &replacement);

static std::vector<XMLEle *> findBlobElements(XMLEle *root);

/* log message in root known to be from device dev to ldir, if any.
 */
static void logDMsg(XMLEle *root, const char *dev);