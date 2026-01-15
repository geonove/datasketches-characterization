# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.

clf;

ddsketch_clds=load('../results/ddsketch_clds_2048_001_rel_rank_error.tsv');
ddsketch_chds=load('../results/ddsketch_chds_2048_001_rel_rank_error.tsv');
td100=load("../results/tdigest_100_rel_rank_error.tsv");
td200=load("../results/tdigest_200_rel_rank_error.tsv");
req10_hra=load("../results/req_sketch_10_HRA_rel_rank_error.tsv");
req40_hra=load("../results/req_sketch_40_HRA_rel_rank_error.tsv");
req10_lra=load("../results/req_sketch_10_LRA_rel_rank_error.tsv");
req40_lra=load("../results/req_sketch_40_LRA_rel_rank_error.tsv");

#p0.99
figure(1);
hold on;
loglog(ddsketch_clds(:,1), ddsketch_clds(:,6), 'linewidth', 2);
loglog(ddsketch_chds(:,1), ddsketch_chds(:,6), 'linewidth', 2);
loglog(td100(:,1), td100(:,6), 'linewidth', 2);
loglog(td200(:,1), td200(:,6), 'linewidth', 2);
loglog(req10_hra(:,1), req10_hra(:,6), 'linewidth', 2);
loglog(req40_hra(:,1), req40_hra(:,6), 'linewidth', 2);

set(gca, 'fontsize', 16);
title 'Rank Error of TDigest, REQ sketch and DDSketch, exponential distribution lambda=1.5, 1000 trials 99 pct'
xlabel 'stream size'
ylabel 'relative quantile error |true - esimate| / estimate, %'
grid minor on
legend(
'DDSketch - Collapsing Lowest Buckets, alpha=0.01, rank 0.99',
'DDSketch - Collapsing Highest Buckets, alpha=0.01, rank 0.99',
'tdigest, k=100, rank 0.99',
'tdigest, k=200, rank 0.99',
'REQ - HRA, k=10, rank 0.99',
'REQ - HRA, k=40, rank 0.99',
'location', 'northeast'
);

#p0.01
%hold on;
figure(2);
hold on;
loglog(ddsketch_clds(:,1), ddsketch_clds(:,2), 'linewidth', 2);
loglog(ddsketch_chds(:,1), ddsketch_chds(:,2), 'linewidth', 2);
loglog(td100(:,1), td100(:,2), 'linewidth', 2);
loglog(td200(:,1), td200(:,2), 'linewidth', 2);
loglog(req10_lra(:,1), req10_lra(:,2), 'linewidth', 2);
loglog(req40_lra(:,1), req40_lra(:,2), 'linewidth', 2);

set(gca, 'fontsize', 16);
title 'Relative Quantile Error of TDigest and DDSketch, exponential distribution lambda=1.5, 1000 trials 99 pct'
xlabel 'stream size'
ylabel 'relative quantile error |true - esimate| / estimate'
grid minor on
legend(
'DDSketch - Collapsing Lowest Buckets, alpha=0.01, rank 0.01',
'DDSketch - Collapsing Highest Buckets, alpha=0.01, rank 0.01',
'tdigest, k=100, rank 0.01',
'tdigest, k=200, rank 0.01',
'REQ - LRA, k=10, rank 0.01',
'REQ - LRA, k=40, rank 0.01',
'location', 'northwest'
);

#p0.5
%hold on;
figure(3);
hold on;
loglog(ddsketch_clds(:,1), ddsketch_clds(:,4), 'linewidth', 2);
loglog(ddsketch_chds(:,1), ddsketch_chds(:,4), 'linewidth', 2);
loglog(td100(:,1), td100(:,4), 'linewidth', 2);
loglog(td200(:,1), td200(:,4), 'linewidth', 2);
loglog(req40_hra(:,1), req40_hra(:,4), 'linewidth', 2);
loglog(req40_lra(:,1), req40_lra(:,4), 'linewidth', 2);

set(gca, 'fontsize', 16);
title 'Relative Quantile Error of TDigest and DDSketch, exponential distribution lambda=1.5, 1000 trials 99 pct'
xlabel 'stream size'
ylabel 'relative quantile error |true - esimate| / estimate'
grid minor on
legend(
'DDSketch - Collapsing Lowest Buckets, alpha=0.01, rank 0.5',
'DDSketch - Collapsing Highest Buckets, alpha=0.01, rank 0.5',
'tdigest, k=100, rank 0.5',
'tdigest, k=200, rank 0.5',
'REQ - HRA, k=40, rank 0.5',
'REQ - LRA, k=40, rank 0.5',
'location', 'northwest'
);

pause();