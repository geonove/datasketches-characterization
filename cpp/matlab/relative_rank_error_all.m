clf;
close all;

output_dir = '../results/rank_accuracy/plots';
mkdir(output_dir);

% # Load data for each distribution
% # Uniform
uni_clds=load('../results/rank_accuracy/uniform/ddsketch_2048_CLDS.tsv');
uni_chds=load('../results/rank_accuracy/uniform/ddsketch_2048_CHDS.tsv');
uni_td=load('../results/rank_accuracy/uniform/tdigest_200.tsv');
uni_req_hra=load('../results/rank_accuracy/uniform/reqsketch_30_HRA.tsv');
uni_req_lra=load('../results/rank_accuracy/uniform/reqsketch_30_LRA.tsv');

% # Exponential
exp_clds=load('../results/rank_accuracy/exponential/ddsketch_2048_CLDS.tsv');
exp_chds=load('../results/rank_accuracy/exponential/ddsketch_2048_CHDS.tsv');
exp_td=load('../results/rank_accuracy/exponential/tdigest_200.tsv');
exp_req_hra=load('../results/rank_accuracy/exponential/reqsketch_30_HRA.tsv');
exp_req_lra=load('../results/rank_accuracy/exponential/reqsketch_30_LRA.tsv');

% # Pareto
par_clds=load('../results/rank_accuracy/pareto/ddsketch_2048_CLDS.tsv');
par_chds=load('../results/rank_accuracy/pareto/ddsketch_2048_CHDS.tsv');
par_td=load('../results/rank_accuracy/pareto/tdigest_200.tsv');
par_req_hra=load('../results/rank_accuracy/pareto/reqsketch_30_HRA.tsv');
par_req_lra=load('../results/rank_accuracy/pareto/reqsketch_30_LRA.tsv');

% # Column indices: 1=N, 2=0.01, 3=0.05, 4=0.5, 5=0.95, 6=0.99

legend_entries = {
  'DDSketch CLDS, a=0.01',
  'DDSketch CHDS, a=0.01',
  't-digest, k=200',
  'REQ HRA, k=30',
  'REQ LRA, k=30'
};

% # rank_cols: column index for each rank
% # rank_labels: label for filenames and titles
rank_cols = [2, 4, 6];
rank_labels = {'0.01', '0.5', '0.99'};

dist_labels = {'uniform', 'exponential', 'pareto'};
dist_titles = {'Uniform(0,1)', 'Exponential(1.5)', 'Pareto(1.5, 1)'};

% # Data arrays indexed by distribution: {uniform, exponential, pareto}
clds = {uni_clds, exp_clds, par_clds};
chds = {uni_chds, exp_chds, par_chds};
td = {uni_td, exp_td, par_td};
req_hra = {uni_req_hra, exp_req_hra, par_req_hra};
req_lra = {uni_req_lra, exp_req_lra, par_req_lra};

fig_num = 1;
for r = 1:length(rank_cols)
  col = rank_cols(r);
  for d = 1:length(dist_labels)
    figure(fig_num);
    hold on;
    loglog(clds{d}(:,1), clds{d}(:,col), 'linewidth', 2);
    loglog(chds{d}(:,1), chds{d}(:,col), 'linewidth', 2);
    loglog(td{d}(:,1), td{d}(:,col), 'linewidth', 2);
    loglog(req_hra{d}(:,1), req_hra{d}(:,col), 'linewidth', 2);
    loglog(req_lra{d}(:,1), req_lra{d}(:,col), 'linewidth', 2);
    set(gca, 'fontsize', 14);
    title(sprintf('Rank Error, %s, rank %s', dist_titles{d}, rank_labels{r}));
    xlabel('stream size');
    ylabel('rank error, %');
    grid minor on;
    legend(legend_entries{:}, 'location', 'northeast');

    filename = sprintf('%s/rank_error_%s_rank_%s.png', output_dir, dist_labels{d}, rank_labels{r});
    print(filename, '-dpng', '-r150');

    fig_num = fig_num + 1;
  end
end

pause;
