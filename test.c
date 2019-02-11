//  test.c
//
//  (c) 2019 Skip Sopscak
//  This code is licensed under MIT license (see LICENSE for details)

#include "string.h"
#include "json.h"

#include <assert.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
    {
    char *s = get_json_string(NULL, 1024);
    ret_json_string(NULL, s);

#ifdef _GNU_SOURCE
    const char *tstrings[] = { 
        "  27.312  ",
        "true",
        "false",
        "null",
        "[]",
        "{}",
        "[1]",
        "[1,2]",
        "[1,2,\"foo\"]",
        "{\"foo\": \"bar\", \"baz\": \"blah\"}",
        "  \" foobar \"  "
    };

    for (int i = 0; i < sizeof(tstrings)/sizeof(tstrings[0]); ++i)
        {
        JSON *json = json_parse_string(tstrings[i]);
        JSON_DATA *data = json_get_root(json);
        if (json_is_string(data))
            printf("\nstring[%s]\n", json_string(data));
        else if (json_is_null(data))
            printf("\nIt's null\n");
        else if (json_is_number(data))
            printf("\nnumber[%f]\n", json_number(data));
        else if (json_is_boolean(data))
            printf("\nboolean[%d]\n", json_boolean(data));
        else if (json_is_object(data))
            printf("\nIt's an object\n");
        else if (json_is_array(data))
            printf("\nIt's an array\n");
        else
            {
            printf("\nUnknown data type\n");
            return -1;
            }
        json_dump(json, stdout);
        printf("\n");
        json_destroy(json);
        }
    const char *big_test="{\"web-app\":{\"servlet\":[{\"servlet-name\":\"cofaxCDS\",\"servlet-class\":\"org.cofax.cds.CDSServlet\",\"init-param\":{\"configGlossary:installationAt\":\"Philadelphia, PA\",\"configGlossary:adminEmail\":\"ksm@pobox.com\",\"configGlossary:poweredBy\":\"Cofax\",\"configGlossary:poweredByIcon\":\"/images/cofax.gif\",\"configGlossary:staticPath\":\"/content/static\",\"templateProcessorClass\":\"org.cofax.WysiwygTemplate\",\"templateLoaderClass\":\"org.cofax.FilesTemplateLoader\",\"templatePath\":\"templates\",\"templateOverridePath\":\"\",\"defaultListTemplate\":\"listTemplate.htm\",\"defaultFileTemplate\":\"articleTemplate.htm\",\"useJSP\":false,\"jspListTemplate\":\"listTemplate.jsp\",\"jspFileTemplate\":\"articleTemplate.jsp\",\"cachePackageTagsTrack\":200,\"cachePackageTagsStore\":200,\"cachePackageTagsRefresh\":60,\"cacheTemplatesTrack\":100,\"cacheTemplatesStore\":50,\"cacheTemplatesRefresh\":15,\"cachePagesTrack\":200,\"cachePagesStore\":100,\"cachePagesRefresh\":10,\"cachePagesDirtyRead\":10,\"searchEngineListTemplate\":\"forSearchEnginesList.htm\",\"searchEngineFileTemplate\":\"forSearchEngines.htm\",\"searchEngineRobotsDb\":\"WEB-INF/robots.db\",\"useDataStore\":true,\"dataStoreClass\":\"org.cofax.SqlDataStore\",\"redirectionClass\":\"org.cofax.SqlRedirection\",\"dataStoreName\":\"cofax\",\"dataStoreDriver\":\"com.microsoft.jdbc.sqlserver.SQLServerDriver\",\"dataStoreUrl\":\"jdbc:microsoft:sqlserver://LOCALHOST:1433;DatabaseName=goon\",\"dataStoreUser\":\"sa\",\"dataStorePassword\":\"dataStoreTestQuery\",\"dataStoreTestQuery\":\"SET NOCOUNT ON;select test='test';\",\"dataStoreLogFile\":\"/usr/local/tomcat/logs/datastore.log\",\"dataStoreInitConns\":10,\"dataStoreMaxConns\":100,\"dataStoreConnUsageLimit\":100,\"dataStoreLogLevel\":\"debug\",\"maxUrlLength\":500}},{\"servlet-name\":\"cofaxEmail\",\"servlet-class\":\"org.cofax.cds.EmailServlet\",\"init-param\":{\"mailHost\":\"mail1\",\"mailHostOverride\":\"mail2\"}},{\"servlet-name\":\"cofaxAdmin\",\"servlet-class\":\"org.cofax.cds.AdminServlet\"},{\"servlet-name\":\"fileServlet\",\"servlet-class\":\"org.cofax.cds.FileServlet\"},{\"servlet-name\":\"cofaxTools\",\"servlet-class\":\"org.cofax.cms.CofaxToolsServlet\",\"init-param\":{\"templatePath\":\"toolstemplates/\",\"log\":1,\"logLocation\":\"/usr/local/tomcat/logs/CofaxTools.log\",\"logMaxSize\":\"\",\"dataLog\":1,\"dataLogLocation\":\"/usr/local/tomcat/logs/dataLog.log\",\"dataLogMaxSize\":\"\",\"removePageCache\":\"/content/admin/remove?cache=pages&id=\",\"removeTemplateCache\":\"/content/admin/remove?cache=templates&id=\",\"fileTransferFolder\":\"/usr/local/tomcat/webapps/content/fileTransferFolder\",\"lookInContext\":1,\"adminGroupID\":4,\"betaServer\":true}}],\"servlet-mapping\":{\"cofaxCDS\":\"/\",\"cofaxEmail\":\"/cofaxutil/aemail/*\",\"cofaxAdmin\":\"/admin/*\",\"fileServlet\":\"/static/*\",\"cofaxTools\":\"/tools/*\"},\"taglib\":{\"taglib-uri\":\"cofax.tld\",\"taglib-location\":\"/WEB-INF/tlds/cofax.tld\"}}}";
    for (int i = 0; i < 1024; ++i)
        {
        char *str_copy = strdup(big_test);
        JSON *json = json_parse_string(str_copy);
        free(str_copy);
        json_destroy(json);
        }

#endif

    FILE *f = fopen("test.json", "r");
    JSON *json = json_parse_file(f);
    fclose(f);
    JSON_DATA *root = json_get_root(json);
    JSON_DATA *d = json_get_data(root, "bands,beatles,lead");
    printf("%s was lead guitar\n", json_string(d));
    d = json_get_data(root, "genres,2");
    printf("%s is the 3rd 'P'\n", json_string(d));
    d = json_get_data(root, "genres");
    JSON_DATA **da = json_array(d);
    printf("The 4 P's:\n");
    while (*da)
        {
        printf("  %s\n", json_string(*da));
        ++da;
        }
    d = json_get_data(root, "bands");
    assert(d);
    d = json_get_data(root, "big_string2");
    assert(d);
    d = json_get_data(root, "bands,devo,bass");
    assert(!d);

    json_destroy(json);

    json = json_parse_file(stdin);
    json_dump(json, stdout);
    printf("\n");

    return 0;
    }
