import numpy as np
import matplotlib.pyplot as plt
import os

output_dir = '../results/rank_accuracy/plots'
os.makedirs(output_dir, exist_ok=True)

base = '../results/rank_accuracy'

# Load data for each distribution
dists = {
    'uniform': 'Uniform(0,1)',
    'exponential': 'Exponential(1.5)',
    'pareto': 'Pareto(1.5, 1)',
}

data = {}
for dist in dists:
    data[dist] = {
        'clds': np.loadtxt(f'{base}/{dist}/ddsketch_2048_CLDS.tsv'),
        'chds': np.loadtxt(f'{base}/{dist}/ddsketch_2048_CHDS.tsv'),
        'td': np.loadtxt(f'{base}/{dist}/tdigest_200.tsv'),
        'req_hra': np.loadtxt(f'{base}/{dist}/reqsketch_30_HRA.tsv'),
        'req_lra': np.loadtxt(f'{base}/{dist}/reqsketch_30_LRA.tsv'),
    }

# Column indices: 0=N, 1=0.01, 2=0.05, 3=0.5, 4=0.95, 5=0.99
ranks = {
    '0.01': 1,
    '0.5': 3,
    '0.99': 5,
}

sketches = [
    ('clds', 'DDSketch CLDS, a=0.01'),
    ('chds', 'DDSketch CHDS, a=0.01'),
    ('td', 't-digest, k=200'),
    ('req_hra', 'REQ HRA, k=30'),
    ('req_lra', 'REQ LRA, k=30'),
]

# Individual plots
for rank_label, col in ranks.items():
    for dist_key, dist_title in dists.items():
        fig, ax = plt.subplots(figsize=(8, 5))
        for sketch_key, sketch_label in sketches:
            d = data[dist_key][sketch_key]
            ax.loglog(d[:, 0], d[:, col], linewidth=2, label=sketch_label)
        ax.set_xlabel('Stream size', fontsize=14)
        ax.set_ylabel('Rank error, %', fontsize=14)
        ax.set_title(f'Rank Error, {dist_title}, rank {rank_label}', fontsize=14)
        ax.legend(fontsize=10)
        ax.grid(True, which='both', linestyle='--', alpha=0.5)
        ax.tick_params(labelsize=12)
        fig.tight_layout()

        filename = f'{output_dir}/rank_error_{dist_key}_rank_{rank_label}.pdf'
        fig.savefig(filename, dpi=300)
        plt.close(fig)
        print(f'Saved {filename}')

# Combined 3x3 plot: rows=ranks, cols=distributions
rank_list = list(ranks.items())
dist_list = list(dists.items())

fig, axes = plt.subplots(3, 3, figsize=(18, 14))

for r, (rank_label, col) in enumerate(rank_list):
    for d, (dist_key, dist_title) in enumerate(dist_list):
        ax = axes[r][d]
        for sketch_key, sketch_label in sketches:
            dat = data[dist_key][sketch_key]
            ax.loglog(dat[:, 0], dat[:, col], linewidth=1.5, label=sketch_label)
        ax.grid(True, which='both', linestyle='--', alpha=0.5)
        ax.tick_params(labelsize=9)
        if r == 0:
            ax.set_title(dist_title, fontsize=13, fontweight='bold')
        if d == 0:
            ax.set_ylabel(f'rank {rank_label}\n\nrank error, %', fontsize=11)
        if r == 2:
            ax.set_xlabel('Stream size', fontsize=11)

# Single legend at the bottom
handles, labels = axes[0][0].get_legend_handles_labels()
fig.legend(handles, labels, loc='lower center', ncol=5, fontsize=11,
           bbox_to_anchor=(0.5, 0.01))

fig.suptitle('Relative Rank Error, 1000 trials, 99th percentile', fontsize=15, fontweight='bold')
fig.tight_layout(rect=[0, 0.05, 1, 0.96])

combined_filename = f'{output_dir}/rank_error_combined_3x3.pdf'
fig.savefig(combined_filename, dpi=300)
plt.close(fig)
print(f'Saved {combined_filename}')

print('Done.')
