Hyperlink-Induced Topic Search (HITS; also known as Hubs and Authorities) is a
link analysis algorithm that rates Web Pages. It was developed by Job Kleinberg, a
professor in the Department of Computer Science at Cornell. The idea behind Hubs and
Authorities stemmed from a particular insight into the creation of web pages when the
Internet was originally forming; that is, certain web pages, known as hubs, served as
large directories that were not actually authoritative in the information that it held, but
were used as compilations of a broad catalog of information that led users directly to
other authoritative pages. In other words, a good hub represented a page that pointed to
many other pages, and a good authority represented a page that was linked by many
different hubs.
The scheme therefore assigns two scores for each page: its authority, which
estimates the value of the content of the page, and its hub value, which estimates the
value of its links to other pages. Authority and hub values are defined in terms of one
another in a mutual recursion. An authority value is computed as the sum of the scaled
hub values that point to that page. A hub value is the sum of the scaled authority values
of the pages it points to. Some implementations also consider the relevance of the linked
pages.
HITS algorithm is in the same spirit as PageRank. They both make use of the link
structure of the Web graph in order to decide the relevance of the pages. The difference is
that unlike the PageRank algorithm, HITS only operates on a small sub-graph (the seed
SQ) from the web graph. This sub-graph is query dependent; whenever we search with a
different query phrase, the seed changes as well. HITS rank the seed nodes according to
their authority and hub weights. The highest ranking pages are displayed to the user by
the query engine.
HITS Algorithm can exploit the plenitude of transistors present on the GPU. The
SIMD architecture of the GPU can help to reduce the complexity of the algorithm. For
any iteration, a single thread will be assigned to a single node on the Web Graph, which
will compute the Hub Score for the node. Similarly, after the calculation of the Hub
Score, the authority score will be computed. The Normalization process can also be
executed in parallel. Hence the above three steps will use the computation strength to
reduce the time required to process the authority pages for the given Web Graph.
Algorithms such as Kleinberg‘s HITS algorithm, the PageRank algorithm of Brin
and Page, and the SALSA algorithm of Lempel and Moran use the link structure of a
network of webpages to assign weights to each page in the network. The weights can then

7
be used to rank the pages as authoritative sources. These algorithms share a common
underpinning; they find a dominant eigenvector of a non-negative matrix that describes
the link structure of the given network and use the entries of this eigenvector as the page
weights. We use this commonality to give a unified treatment, proving the existence of
the required eigenvector for the PageRank, HITS, and SALSA algorithms, the uniqueness
of the PageRank eigenvector, and the convergence of the algorithms to these
eigenvectors. However, we show that the HITS and SALSA eigenvectors need not be
unique. We examine how the initialization of the algorithms affects the final weightings
produced. We give examples of networks that lead the HITS and SALSA algorithms to
return non-unique or non-intuitive rankings. We characterize all such networks, in terms
of the connectivity of the related HITS authority graph. We propose a modification, to
HITS, using non-uniform distribution of authority and hub value. We prove that HITS
returns a unique ranking, so long as the network is weakly connected.