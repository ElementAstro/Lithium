#include "xml_util.hpp"

#include <cstring>

#include "time.hpp"
#include "hydrogen_server.hpp"

int xmlReplacementMapFind(void *self, XMLEle *source, XMLEle **replace)
{
    auto map = (const std::unordered_map<XMLEle *, XMLEle *> *)self;
    auto idx = map->find(source);
    if (idx == map->end())
    {
        return 0;
    }
    *replace = (XMLEle *)idx->second;
    return 1;
}

XMLEle *cloneXMLEleWithReplacementMap(XMLEle *root, const std::unordered_map<XMLEle *, XMLEle *> &replacement)
{
    return cloneXMLEle(root, &xmlReplacementMapFind, (void *)&replacement);
}


std::vector<XMLEle *> findBlobElements(XMLEle *root)
{
    std::vector<XMLEle *> result;
    for (auto ep = nextXMLEle(root, 1); ep; ep = nextXMLEle(root, 0))
    {
        if (strcmp(tagXMLEle(ep), "oneBLOB") == 0)
        {
            result.push_back(ep);
        }
    }
    return result;
}

/* log message in root known to be from device dev to ldir, if any.
 */
void logDMsg(XMLEle *root, const char *dev)
{
    char stamp[64];
    char logfn[1024];
    const char *ts, *ms;
    FILE *fp;

    /* get message, if any */
    ms = findXMLAttValu(root, "message");
    if (!ms[0])
        return;

    /* get timestamp now if not provided */
    ts = findXMLAttValu(root, "timestamp");
    if (!ts[0])
    {
        hydrogen_tstamp(stamp);
        ts = stamp;
    }

    /* append to log file, name is date portion of time stamp */
    sprintf(logfn, "%s/%.10s.islog", ldir, ts);
    fp = fopen(logfn, "a");
    if (!fp)
        return; /* oh well */
    fprintf(fp, "%s: %s: %s\n", ts, dev, ms);
    fclose(fp);
}