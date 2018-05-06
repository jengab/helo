#include <sstream>
#include <lest/lest.hpp>
#include <trompeloeil.hpp>
#include "cluster.h"
#include "LogParserMock.h"

static inline Cluster genCluster(const std::wstring& fileContent, const std::wstring& regexp) {
    LogParserMock parser(0, regexp);

    std::wstringstream fileObj(fileContent);
    fileObj >> parser;
    return Cluster(parser.getContent(), parser.getDictionary());
}

static inline std::wstring getTemplateMsg(const ArrayOfWords& clusterTemplate) {
    std::wstringstream templateMsg;
    for (size_t i=0; i<clusterTemplate.size(); ++i) {
        templateMsg << clusterTemplate[i]->TokenString;
        if (i<clusterTemplate.size()-1) {
            templateMsg << " ";
        }
    }
    return templateMsg.str();
}

static const lest::test _clusterSuite[] {
    CASE("split: exception is thrown if cluster is not filled") {
        Cluster clust = genCluster(L"A B\n\
                A B D\n\
                A B C\n\
                A B\n\
                A B\n", L"[\\s]+");
        ListOfClusters workList;
        EXPECT_THROWS_AS(clust.Split(workList), std::invalid_argument);
    },
    CASE("split: Same value in each column cannot be split") {
        Cluster clust = genCluster(L"A B C\n\
                A B C\n\
                A B C\n\
                A B C\n\
                A B C\n", L"[\\s]+");
        ListOfClusters workList;
        EXPECT_THROWS_AS(clust.Split(workList), std::invalid_argument);
    },
    CASE("split: Cluster is split to two others at variable column") {
        Cluster clust = genCluster(L"A B C\n\
                A B C\n\
                A B C\n\
                A C C\n\
                A C C\n\
                A C C\n", L"[\\s]+");
        ListOfClusters workList;
        clust.Split(workList);
        EXPECT(workList.size() == 2u);
    },
    CASE("split: split is done at most variable column") {
        Cluster clust = genCluster(L"A A A\n\
                A B B\n\
                A C C\n\
                A D D\n\
                A E B\n\
                A F C\n", L"[\\s]+");
        ListOfClusters workList;
        clust.Split(workList);
        EXPECT(workList.size() == 4u);
    },
    CASE("split: Cluster is not split at number") {
        Cluster clust = genCluster(L"A 1 C\n\
                B 2 C\n\
                C 3 C\n\
                A 4 C\n\
                B 5 C\n\
                C 6 C\n", L"[\\s]+");
        ListOfClusters workList;
        clust.Split(workList);
        EXPECT(workList.size() == 3u);
    },
    CASE("split: In mixed columns all numbers get into the same cluster") {
        Cluster clust = genCluster(L"A X C\n\
                A X C\n\
                A 1 C\n\
                A X C\n\
                A 3 C\n\
                A X C\n", L"[\\s]+");
        ListOfClusters workList;
        clust.Split(workList);
        EXPECT(workList.size() == 2u);
        for (const Cluster& actCluster : workList) {
            std::wstring templateStr = getTemplateMsg(actCluster.getTemplate());
            EXPECT((templateStr == L"A X C" || templateStr == L"A +d C"));
        }
    },
    CASE("split: Longer rows are put to the same cluster") {
        Cluster clust = genCluster(L"A B C A\n\
                A B C\n\
                A B C B\n\
                A B C\n\
                A B C C D\n\
                A B C D D\n", L"[\\s]+");
        ListOfClusters workList;
        clust.Split(workList);
        bool abcIsOneCluster = false;
        for (const Cluster& actCluster : workList) {
            std::wstring templateStr = getTemplateMsg(actCluster.getTemplate());
            abcIsOneCluster |= templateStr==L"A B C";
        }
        EXPECT(abcIsOneCluster);
    },
    CASE("split: Numeric values are not taken into account when calculation split position") {
        Cluster clust = genCluster(L"A B C\n\
                A B C\n\
                A 1 D\n\
                A 1 D\n\
                A 1 E\n\
                A B E\n", L"[\\s]+");
        ListOfClusters workList;
        clust.Split(workList);
        EXPECT(workList.size() == 3u);
    },
    CASE("getTemplate: Variable letters are compressed to asterix") {
        Cluster clust = genCluster(L"A B C\n\
                A B C\n\
                A B C\n\
                A C C\n\
                A C C\n\
                A C C\n", L"[\\s]+");
        EXPECT(getTemplateMsg(clust.getTemplate()) == L"A * C");
    },
    CASE("getTemplate: Variable integers are compressed to '+d'") {
        Cluster clust = genCluster(L"A 1 C\n\
                A 2 C\n\
                A 3 C\n\
                A 4 C\n\
                A 5 C\n\
                A 5 C\n", L"[\\s]+");
        EXPECT(getTemplateMsg(clust.getTemplate()) == L"A +d C");
    },
    CASE("getTemplate: Different line endings are compressed to '+n'") {
        Cluster clust = genCluster(L"A B C\n\
                A B C\n\
                A B C D\n\
                A B C D E\n\
                A B C D E F\n\
                A B C D E F\n", L"[\\s]+");
        EXPECT(getTemplateMsg(clust.getTemplate()) == L"A B C +n");
    },
    CASE("compressToTemplate: Cluster contains only 1 line after compression") {
        Cluster clust = genCluster(L"A B C\n\
                A C C\n\
                A D C\n\
                A B C\n\
                A C C",L"[\\s]+");
        clust.compressToTemplate();
        EXPECT(clust.getContent()->size() == 1u);
    },
    CASE("compressToTemplate: Cluster contains the template") {
        Cluster clust = genCluster(L"A B C\n\
                A C C\n\
                A D C\n\
                A B C\n\
                A C C",L"[\\s]+");
        clust.compressToTemplate();
        EXPECT(getTemplateMsg(clust.getContent()->front()) == L"A * C");
    },
    CASE("getGoodness: Empty cluster's goodness is 1") {
        Cluster clust = genCluster(L"", L"[\\s]+");
        EXPECT(clust.getGoodness() == 1.0);
    },
    CASE("getGoodness: Goodness is 0 if no common word") {
        Cluster clust = genCluster(L"A B C\n\
                D E F\n\
                G H I\n\
                J K L M\n\
                N O P Q\n\
                R S T U V\n\
                W X Y Z\n\
                A B C\n", L"[\\s]+");
        EXPECT(clust.getGoodness() == 0.0);
    },
    CASE("getGoodness: Goodness is 0.5 if half of the words common") {
        Cluster clust = genCluster(L"A B C D\n\
                D B F D\n\
                G B I D\n\
                A B C D\n", L"[\\s]+");
        EXPECT(clust.getGoodness() == 0.5);
    },
    CASE("getGoodness: Goodness is calculated based on average line length") {
        Cluster clust = genCluster(L"A B C D E\n\
                D B F D\n\
                G B I D E\n\
                A B C D\n", L"[\\s]+");
        EXPECT(clust.getGoodness() == 2.0/4.5);
    },
    CASE("getGoodness: Goodness is 1 for identical lines") {
        Cluster clust = genCluster(L"A B C\n\
                A B C\n\
                A B C\n\
                A B C\n", L"[\\s]+");
        EXPECT(clust.getGoodness() == 1.0);
    },
    CASE("getGoodness: '+d' is treated as common token") {
        Cluster clust1 = genCluster(L"A +d C D\n", L"[\\s]+");
        Cluster clust2 = genCluster(L"X +d C E\n", L"[\\s]+");
        EXPECT(clust1.getGoodness(clust2) == 0.5);
    },
    CASE("getGoodness: '+n' is treated as common token at end of line") {
        Cluster clust1 = genCluster(L"A B C D +n\n", L"[\\s]+");
        Cluster clust2 = genCluster(L"X B C E +n\n", L"[\\s]+");
        EXPECT(clust1.getGoodness(clust2) == 3.0/5.0);
    },
    CASE("getGoodness: '+n' matches exactly once with longer lines") {
        Cluster clust1 = genCluster(L"A B C +n", L"[\\s]+");
        Cluster clust2 = genCluster(L"A X C Y E Z", L"[\\s]+");
        EXPECT(clust1.getGoodness(clust2) == 3.0/5.0);
    },
    CASE("getGoodness: Goodness for same cluster is 1") {
        Cluster clust = genCluster(L"A B C", L"[\\s]+");
        EXPECT(clust.getGoodness(clust) == 1.0);
    },
    CASE("getGoodness: goodness is symetic") {
        Cluster clust1 = genCluster(L"A B C D\n", L"[\\s]+");
        Cluster clust2 = genCluster(L"X B C E\n", L"[\\s]+");
        EXPECT(clust1.getGoodness(clust2) == clust2.getGoodness(clust1));
    },
    CASE("join: The parameter cluster is empty after join") {
        Cluster clust1 = genCluster(L"A B C", L"[\\s]+");
        Cluster clust2 = genCluster(L"A B C", L"[\\s]+");
        clust1.join(clust2);
        EXPECT(clust2.getContent()->size() == 0u);
    },
    CASE("join: Same templates are joined into same") {
        Cluster clust1 = genCluster(L"A B C", L"[\\s]+");
        Cluster clust2 = genCluster(L"A B C", L"[\\s]+");
        clust1.join(clust2);
        EXPECT(getTemplateMsg(clust1.getContent()->front()) == L"A B C");
    },
    CASE("join: Different line endings are joined into '+n'") {
        Cluster clust1 = genCluster(L"A B C", L"[\\s]+");
        Cluster clust2 = genCluster(L"A B C D E", L"[\\s]+");
        clust1.join(clust2);
        EXPECT(getTemplateMsg(clust1.getContent()->front()) == L"A B C +n");
    },
    CASE("join: Different numbers are joined into '+d'") {
        Cluster clust1 = genCluster(L"A 1 C", L"[\\s]+");
        Cluster clust2 = genCluster(L"A 0xa C", L"[\\s]+");
        clust1.join(clust2);
        EXPECT(getTemplateMsg(clust1.getContent()->front()) == L"A +d C");
    },
    CASE("join: Different word tokens are joined into '*'") {
        Cluster clust1 = genCluster(L"A X C D", L"[\\s]+");
        Cluster clust2 = genCluster(L"A B C", L"[\\s]+");
        clust1.join(clust2);
        EXPECT(getTemplateMsg(clust1.getContent()->front()) == L"A * C +n");
    },
    CASE("join: Number constants are not replaced with '+d'") {
        Cluster clust1 = genCluster(L"A 1 B * +n", L"[\\s]+");
        Cluster clust2 = genCluster(L"A 1 C +n", L"[\\s]+");
        clust1.join(clust2);
        EXPECT(getTemplateMsg(clust1.getContent()->front()) == L"A 1 * +n");
    },
    CASE("join: After '+n' in second cluster processing stops") {
        Cluster clust1 = genCluster(L"A 1 C D E F", L"[\\s]+");
        Cluster clust2 = genCluster(L"A 2 C X +n", L"[\\s]+");
        clust1.join(clust2);
        EXPECT(getTemplateMsg(clust1.getContent()->front()) == L"A +d C * +n");
    },
    CASE("join: After '+n' in first cluster processing stops") {
        Cluster clust1 = genCluster(L"A 2 C X +n", L"[\\s]+");
        Cluster clust2 = genCluster(L"A 1 C D E F", L"[\\s]+");
        clust1.join(clust2);
        EXPECT(getTemplateMsg(clust1.getContent()->front()) == L"A +d C * +n");
    },
    CASE("join: Join gives same result right to left as reverse") {
        Cluster clust1 = genCluster(L"A 1 C", L"[\\s]+");
        Cluster clust2 = genCluster(L"A 2 C D E", L"[\\s]+");
        Cluster tmpClust1 = genCluster(L"A 1 C", L"[\\s]+");
        Cluster tmpClust2 = genCluster(L"A 2 C D E", L"[\\s]+");
        clust1.join(clust2);
        tmpClust2.join(tmpClust1);
        EXPECT(getTemplateMsg(clust1.getContent()->front()) == getTemplateMsg(tmpClust2.getContent()->front()));
    }
};

extern const lest::tests clusterSuite(_clusterSuite, _clusterSuite + sizeof(_clusterSuite) / sizeof(*_clusterSuite));
