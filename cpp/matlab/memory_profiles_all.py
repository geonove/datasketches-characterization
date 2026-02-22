import numpy as np
import matplotlib.pyplot as plt
import os

output_dir = '../results/memory_profiles/plots'
os.makedirs(output_dir, exist_ok=True)

base = '../results/memory_profiles'

dists = {
    'uniform': 'Uniform(0,1)',
    'exponential': 'Exponential(1.5)',
    'pareto': 'Pareto(1.5, 1)',
}

sketches = [
    ('ddsketch_2048_CLDS', 'DDSketch CLDS, a=0.01', 'o'),
    ('ddsketch_2048_CHDS', 'DDSketch CHDS, a=0.01', 's'),
    ('tdigest_200', 't-digest, k=200', '^'),
    ('reqsketch_30_HRA', 'REQ HRA, k=30', 'D'),
    ('reqsketch_30_LRA', 'REQ LRA, k=30', 'v'),
]

# Memory TSV columns: 0=stream_size, 1=num_trials, 2..10=quantiles (min,0.01,0.05,0.25,0.5,0.75,0.95,0.99,max)
# Use column 6 = median (0.5 quantile)
MEDIAN_COL = 6

data = {}
for dist in dists:
    data[dist] = {}
    for fname, label, marker in sketches:
        data[dist][fname] = np.loadtxt(f'{base}/{dist}/{fname}.tsv')

# Individual plots per distribution
for dist_key, dist_title in dists.items():
    fig, ax = plt.subplots(figsize=(8, 5))
    for fname, label, marker in sketches:
        d = data[dist_key][fname]
        ax.semilogx(d[:, 0], d[:, MEDIAN_COL] / 1024, marker=marker, linestyle='-',
                     linewidth=2, markersize=8, label=label)
    ax.set_xlabel('Stream size', fontsize=14)
    ax.set_ylabel('Memory (KB)', fontsize=14)
    ax.set_title(f'Memory Footprint, {dist_title}', fontsize=14)
    ax.legend(fontsize=10)
    ax.grid(True, which='both', linestyle='--', alpha=0.5)
    ax.tick_params(labelsize=12)
    fig.tight_layout()

    filename = f'{output_dir}/memory_{dist_key}.pdf'
    fig.savefig(filename, dpi=300)
    plt.close(fig)
    print(f'Saved {filename}')

# Combined 1x3 plot
fig, axes = plt.subplots(1, 3, figsize=(18, 5))

for d, (dist_key, dist_title) in enumerate(dists.items()):
    ax = axes[d]
    for fname, label, marker in sketches:
        dat = data[dist_key][fname]
        ax.semilogx(dat[:, 0], dat[:, MEDIAN_COL] / 1024, marker=marker, linestyle='-',
                     linewidth=1.5, markersize=6, label=label)
    ax.grid(True, which='both', linestyle='--', alpha=0.5)
    ax.tick_params(labelsize=9)
    ax.set_title(dist_title, fontsize=13, fontweight='bold')
    ax.set_xlabel('Stream size', fontsize=11)
    if d == 0:
        ax.set_ylabel('Memory (KB)', fontsize=11)

handles, labels = axes[0].get_legend_handles_labels()
fig.legend(handles, labels, loc='lower center', ncol=5, fontsize=11,
           bbox_to_anchor=(0.5, 0.01))

fig.suptitle('Memory Footprint (median), 16 trials', fontsize=15, fontweight='bold')
fig.tight_layout(rect=[0, 0.08, 1, 0.93])

combined_filename = f'{output_dir}/memory_combined_1x3.pdf'
fig.savefig(combined_filename, dpi=300)
plt.close(fig)
print(f'Saved {combined_filename}')

print('Done.')