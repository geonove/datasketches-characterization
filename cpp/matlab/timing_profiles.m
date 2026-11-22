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

% Timing in nanoseconds, averaged over trials. The update time is per item.
% Columns: 1 = stream size, 2 = trials, 3 = build, 4 = update, 5 = get_quantile, 6 = get_rank,
% 7 = serialize, 8 = deserialize, 9 = serialized size in bytes.
plot_metric('timing_profiles', 4, 'update_time', 'update time per item, ns', true);
plot_metric('timing_profiles', 5, 'quantile_time', 'get\_quantile time, ns', true);
plot_metric('timing_profiles', 6, 'rank_time', 'get\_rank time, ns', true);
plot_metric('timing_profiles', 7, 'serialize_time', 'serialize time, ns', true);
plot_metric('timing_profiles', 8, 'deserialize_time', 'deserialize time, ns', true);
plot_metric('timing_profiles', 9, 'serialized_size', 'serialized size, bytes', true);
