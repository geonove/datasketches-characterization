import numpy as np
import matplotlib.pyplot as plt
import os

output_dir = '../results/timing_profiles/plots'
os.makedirs(output_dir, exist_ok=True)

base = '../results/timing_profiles'

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

# TSV columns (first row is header):
# 0=Stream, 1=Trials, 2=Build, 3=Update, 4=Quantile, 5=Rank, 6=Serialize, 7=Deserialize, 8=Size
metrics = {
    'construct': ('Sketch creation ns', 2),
    'update': ('Update (ns/item)', 3),
    'get_quantile': ('Get Quantile (ns/query)', 4),
    'get_rank': ('Get Rank (ns/query)', 5),
    'serialize': ('Serialize (ns)', 6),
    'deserialize': ('Deserialize (ns)', 7),
    'size': ('Serialized bytes size', 8)
}

data = {}
for dist in dists:
    data[dist] = {}
    for fname, label, marker in sketches:
        data[dist][fname] = np.loadtxt(f'{base}/{dist}/{fname}.tsv', skiprows=1)

# Individual plots: one per metric per distribution
for metric_key, (metric_title, col) in metrics.items():
    for dist_key, dist_title in dists.items():
        fig, ax = plt.subplots(figsize=(8, 5))
        for fname, label, marker in sketches:
            d = data[dist_key][fname]
            ax.semilogx(d[:, 0], d[:, col], marker=marker, linestyle='-',
                         linewidth=2, markersize=8, label=label)
        ax.set_xlabel('Stream size', fontsize=14)
        ax.set_ylabel(metric_title, fontsize=14)
        ax.set_title(f'{metric_title}, {dist_title}', fontsize=14)
        ax.legend(fontsize=10)
        ax.grid(True, which='both', linestyle='--', alpha=0.5)
        ax.tick_params(labelsize=12)
        fig.tight_layout()

        filename = f'{output_dir}/timing_{metric_key}_{dist_key}.pdf'
        fig.savefig(filename, dpi=300)
        plt.close(fig)
        print(f'Saved {filename}')

# Combined plots: one figure per metric, 1x3 (distributions as columns)
for metric_key, (metric_title, col) in metrics.items():
    fig, axes = plt.subplots(1, 3, figsize=(18, 5))

    for d_idx, (dist_key, dist_title) in enumerate(dists.items()):
        ax = axes[d_idx]
        for fname, label, marker in sketches:
            dat = data[dist_key][fname]
            ax.semilogx(dat[:, 0], dat[:, col], marker=marker, linestyle='-',
                         linewidth=1.5, markersize=6, label=label)
        ax.grid(True, which='both', linestyle='--', alpha=0.5)
        ax.tick_params(labelsize=9)
        ax.set_title(dist_title, fontsize=13, fontweight='bold')
        ax.set_xlabel('Stream size', fontsize=11)
        if d_idx == 0:
            ax.set_ylabel(metric_title, fontsize=11)

    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(handles, labels, loc='lower center', ncol=5, fontsize=11,
               bbox_to_anchor=(0.5, 0.01))

    fig.suptitle(f'{metric_title}, 256 trials', fontsize=15, fontweight='bold')
    fig.tight_layout(rect=[0, 0.08, 1, 0.93])

    combined_filename = f'{output_dir}/timing_{metric_key}_combined_1x3.pdf'
    fig.savefig(combined_filename, dpi=300)
    plt.close(fig)
    print(f'Saved {combined_filename}')

print('Done.')