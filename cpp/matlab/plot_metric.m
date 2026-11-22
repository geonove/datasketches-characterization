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

% Plots one column of the characterization results for every distribution.
%
% Results are read from ../results/<profile_dir>/<distribution>/<sketch>.tsv,
% where <distribution> is uniform, exponential or pareto and <sketch> is one of
% ddsketch_2048_CLDS, ddsketch_2048_CHDS, tdigest_200, reqsketch_30_HRA, reqsketch_30_LRA.
% Missing files are skipped. Figures are written to ../results/<profile_dir>/plots.
%
% col        column of the tsv file to plot (1 is the first column, the x axis)
% name       prefix of the output file names
% ylabel_txt label of the y axis
% log_y      true for a logarithmic y axis
function plot_metric(profile_dir, col, name, ylabel_txt, log_y)
  dists = {'uniform', 'exponential', 'pareto'};
  dist_titles = {'Uniform(0,1)', 'Exponential(1.5)', 'Pareto(1.5, 1)'};
  sketches = {'ddsketch_2048_CLDS', 'ddsketch_2048_CHDS', 'tdigest_200', 'reqsketch_30_HRA', 'reqsketch_30_LRA'};
  labels = {'DDSketch CLDS, a=0.01', 'DDSketch CHDS, a=0.01', 't-digest, k=200', 'REQ HRA, k=30', 'REQ LRA, k=30'};

  out_dir = sprintf('../results/%s/plots', profile_dir);
  if ~exist(out_dir, 'dir')
    mkdir(out_dir);
  end

  for d = 1:length(dists)
    fig = figure('visible', 'off');
    hold on;
    used = {};
    for s = 1:length(sketches)
      file = sprintf('../results/%s/%s/%s.tsv', profile_dir, dists{d}, sketches{s});
      if ~exist(file, 'file')
        continue;
      end
      data = load(file);
      if log_y
        loglog(data(:,1), data(:,col), 'linewidth', 2);
      else
        semilogx(data(:,1), data(:,col), 'linewidth', 2);
      end
      used{end+1} = labels{s};
    end
    if isempty(used)
      close(fig);
      continue;
    end
    set(gca, 'fontsize', 14);
    title(sprintf('%s, %s', strrep(name, '_', ' '), dist_titles{d}));
    xlabel('stream size');
    ylabel(ylabel_txt);
    grid on;
    grid minor;
    legend(used, 'location', 'northeast');
    print(fig, sprintf('%s/%s_%s.png', out_dir, name, dists{d}), '-dpng');
    close(fig);
  end
end
