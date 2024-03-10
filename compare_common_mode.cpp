struct APVAddress
{
    int crate_id, mpd_id, adc_ch;

    APVAddress():crate_id(-1), mpd_id(-1), adc_ch(-1)
    {}

    APVAddress(const APVAddress &addr):crate_id(addr.crate_id),
    mpd_id(addr.mpd_id), adc_ch(addr.adc_ch)
    {}

    APVAddress & operator=(const APVAddress &addr) {
        crate_id = addr.crate_id, mpd_id = addr.mpd_id, adc_ch = addr.adc_ch;
        return *this;
    };

    bool operator==(const APVAddress &addr) const
    {
        return (crate_id == addr.crate_id) && (mpd_id == addr.mpd_id) && (adc_ch == addr.adc_ch);
    }
};

namespace std {
    template<> struct hash<APVAddress>
    {
        std::size_t operator()(const APVAddress &k) const
        {
            return ( (k.adc_ch & 0xf)
                    | ((k.mpd_id & 0x7f) << 4)
                    | ((k.crate_id & 0xff)<<11)
                   );
        }
    };
}

// obsolete
unordered_map<APVAddress, TH1F*> GetPedestalDistribution(const char* file)
{
    unordered_map<APVAddress, TH1F*> res;

    fstream f(file, iostream::in);
    if(!f.is_open()) cout<<"Error: cannot open file: "<<file<<endl;

    return res;
}

// a helper
void draw_pad(TH1F* h1, TH1F* h2, TCanvas* c, int npad, int pos)
{
    c->cd(npad);
    gStyle -> SetOptStat(1);
    h1 -> SetLineColor(2);
    h1 -> SetLineWidth(1);
    h1 -> Draw();
    gPad -> Update();

    h2 -> SetLineColor(4);
    h2 -> SetLineWidth(1);
    h2 -> Draw();
    gPad -> Update();

    THStack *hs = new THStack();
    hs -> SetTitle(h1->GetTitle());
    hs -> Add(h1);
    hs -> Add(h2);

    hs -> Draw("nostack");
    hs -> GetXaxis() -> SetTitle(h1->GetXaxis()->GetTitle());
    hs -> GetYaxis() -> SetTitle(h1->GetYaxis()->GetTitle());
    hs -> GetXaxis() -> SetRangeUser(2000, 3500);

    TLegend *leg = new TLegend(0.6, 0.64, 0.88, 0.85);
    leg -> AddEntry(h1, "old divider run 876", "lp");
    leg -> AddEntry(h2, "new divider run 881", "lp");
    leg -> Draw();

    // draw stats
    gPad -> Update();
    TPaveStats* st1 = (TPaveStats*)h1 -> GetListOfFunctions() -> FindObject("stats");
    st1 -> GetLineWith("Mean") -> SetTextColor(2);
    st1 -> GetLineWith("Entries") -> SetTextColor(2);
    st1 -> GetLineWith("Std Dev") -> SetTextColor(2);
    TPaveStats* st2 = (TPaveStats*)h2 -> FindObject("stats");

    if( pos == 0) // noise
    {
        st1 -> SetX1NDC(0.2); st1 -> SetY1NDC(0.52);
        st1 -> SetX2NDC(0.5); st1 -> SetY2NDC(0.68);
        st1 -> Draw();

        st2 -> SetX1NDC(0.2); st2 -> SetY1NDC(0.72);
        st2 -> SetX2NDC(0.5); st2 -> SetY2NDC(0.88);
        st2 -> Draw();
    }
    gPad -> Modified();
    gPad -> Update();
}

void compare_common_mode(const char *file1="../database/CommonModeRange_876.root",
        const char* file2="../database/CommonModeRange_881.root")
{
    TFile *f1 = new TFile(file1);
    TFile *f2 = new TFile(file2);

    // set up canvas
    TCanvas *c = new TCanvas("c", "noise", 1900, 1000);
    c -> Divide(4, 4);

    for(int i=0; i<16; i++) {
        // noise
        TH1F *h1 = (TH1F*) f1 -> Get(Form("h_CM_crate_%d_mpd_%d_adc_%d", 2, 0, i));
        TH1F *h2 = (TH1F*) f2 -> Get(Form("h_CM_crate_%d_mpd_%d_adc_%d", 2, 0, i));
        draw_pad(h1, h2, c, i+1, 0);
    }

    c -> Print("common_mode_compare.pdf");
}
