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

% Relative quantile error, |estimate - true| / true, 99th percentile over 1000 trials.
% Columns: 1 = N, 2..6 = quantile levels 0.01, 0.05, 0.5, 0.95, 0.99.
levels = [0.01, 0.5, 0.99];
cols = [2, 4, 6];
for i = 1:length(levels)
  plot_metric('value_accuracy', cols(i), sprintf('relative_quantile_error_level_%g', levels(i)), ...
    'relative quantile error', true);
end
