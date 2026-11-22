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

% Merge timing in nanoseconds, averaged over trials.
% Columns: 1 = stream size, 2 = trials, 3 = build, 4 = update, 5 = merge, 6 = serialized size in bytes.
plot_metric('merge_timing_profile', 5, 'merge_time', 'merge time per sketch, ns', true);
plot_metric('merge_timing_profile', 6, 'merged_serialized_size', 'serialized size, bytes', true);
