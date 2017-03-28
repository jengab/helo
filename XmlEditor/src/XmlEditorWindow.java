import java.awt.EventQueue;

import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.border.EmptyBorder;
import javax.swing.filechooser.FileNameExtensionFilter;
import javax.swing.JLabel;
import java.awt.Font;

import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.JSpinner;
import javax.swing.SpinnerNumberModel;
import javax.swing.JTextField;
import javax.swing.JMenuBar;
import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;
import java.io.File;
import java.util.regex.Pattern;
import java.util.regex.PatternSyntaxException;


public class XmlEditorWindow extends JFrame {
	private static final long serialVersionUID = 1L;
	private JPanel contentPane;
	private JTextField LocalStr;
	private JSpinner MergeTrsh;
	private JSpinner HeaderLen;
	private JSpinner PortNo;
	private JMenuItem mntmSave;
	private File OpenedFile=null;
	
	private static final String onlineTag="online";
	private static final String mergeTag="MergeLimit";
	private static final String HeaderTag="HeaderLen";
	private static final String locTag="UsedLocal";
	private static final String portTag="Port";
	private static final String DBPathTag="DBPath";
	private static final String LogPathTag="LogPath";
	private static final String RegexTag="RegExp";
	private static final String WindowTitle="HELO settings";
	private JTextField DBPath;
	private JTextField LogPath;
	private JTextField RegExpr;
	
	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					XmlEditorWindow frame = new XmlEditorWindow();
					frame.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	/**
	 * Create the frame.
	 */
	public XmlEditorWindow() {
		setResizable(false);
		setTitle(WindowTitle);
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setBounds(100, 100, 396, 281);
		
		JMenuBar menuBar = new JMenuBar();
		setJMenuBar(menuBar);
		
		JMenu mnFile = new JMenu("File");
		menuBar.add(mnFile);
		
		JMenuItem mntmOpen = new JMenuItem("Open");
		mntmOpen.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				try{
					JFileChooser chooser=new JFileChooser();
					chooser.setFileFilter(new FileNameExtensionFilter("XML file","xml"));
					int ret=chooser.showOpenDialog(contentPane);
					if(ret==JFileChooser.APPROVE_OPTION){
						OpenedFile=chooser.getSelectedFile();
					}
					else{
						return;
					}
					
					DocumentBuilderFactory bfact=DocumentBuilderFactory.newInstance();
					DocumentBuilder builder=bfact.newDocumentBuilder();
					Document doc=builder.parse(OpenedFile);
					setTitle(WindowTitle+" - "+OpenedFile.getName());
					doc.getDocumentElement().normalize();
					
					Node onlineSettings=doc.getElementsByTagName(onlineTag).item(0);
					Node mergeNode=getChildByName(onlineSettings,mergeTag);
					MergeTrsh.setValue(Double.parseDouble(getAttribute(mergeNode,"value")));
					Node portNode=getChildByName(onlineSettings,portTag);
					PortNo.setValue(Integer.parseInt(getAttribute(portNode,"value")));
					Node HeaderNode=getChildByName(onlineSettings,HeaderTag);
					HeaderLen.setValue(Integer.parseInt(getAttribute(HeaderNode,"value")));
					Node LocNode=getChildByName(onlineSettings,locTag);
					LocalStr.setText(getAttribute(LocNode,"value"));
					Node DBPathNode=getChildByName(onlineSettings,DBPathTag);
					DBPath.setText(getAttribute(DBPathNode,"value"));
					Node LogPathNode=getChildByName(onlineSettings,LogPathTag);
					LogPath.setText(getAttribute(LogPathNode,"value"));
					Node RegExpNode=getChildByName(onlineSettings,RegexTag);
					RegExpr.setText(getAttribute(RegExpNode, "value"));
				}
				catch(Exception ex){
					ex.printStackTrace();
				}
			}
		});
		mnFile.add(mntmOpen);
		
		mntmSave = new JMenuItem("Save");
		mntmSave.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				try{
					SaveFile();
				}
				catch(Exception e){
					e.printStackTrace();
				}
				
			}
		});
		
		mnFile.add(mntmSave);
		
		JMenuItem mntmSaveAs = new JMenuItem("Save As");
		mntmSaveAs.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e) {
				OpenedFile=null;
				try{
					SaveFile();
				}
				catch(Exception ex){
					ex.printStackTrace();
				}
			}
		});
		mnFile.add(mntmSaveAs);
		contentPane = new JPanel();
		contentPane.setToolTipText("<html>\r\nThe name of the used localization<br>\r\nto read the input files, and finally to<br>\r\nwrite the output. For all possible values<br>\r\nSee \"locale -a\" on POSIX systems<br>\r\n</html>");
		contentPane.setBorder(new EmptyBorder(5, 5, 5, 5));
		setContentPane(contentPane);
		contentPane.setLayout(null);
		
		PortNo = new JSpinner();
		PortNo.setModel(new SpinnerNumberModel(514, 0, 65535, 1));
		PortNo.setBounds(161, 9, 65, 20);
		contentPane.add(PortNo);
		
		JLabel lblListenPortOf = new JLabel("Listen port of the server:");
		lblListenPortOf.setFont(new Font("Tahoma", Font.PLAIN, 11));
		lblListenPortOf.setToolTipText("On this port waits the online algorithm for connections.");
		lblListenPortOf.setBounds(10, 12, 148, 14);
		contentPane.add(lblListenPortOf);
		
		JLabel lblMergeGoodnessThreshold_1 = new JLabel("Merge goodness threshold: ");
		lblMergeGoodnessThreshold_1.setFont(new Font("Tahoma", Font.PLAIN, 11));
		lblMergeGoodnessThreshold_1.setToolTipText("<html>\r\nThis value must be between 0 and 1, <br>\r\nand it sets a goodness value limit that  <br>\r\nshould be applied when the algorithm <br>\r\n merges two clusters in its final stage. <br>\r\n</html>");
		lblMergeGoodnessThreshold_1.setBounds(10, 39, 170, 14);
		contentPane.add(lblMergeGoodnessThreshold_1);
		
		MergeTrsh = new JSpinner();
		MergeTrsh.setBounds(186, 41, 40, 20);
		MergeTrsh.setModel(new SpinnerNumberModel(0.4,0.0,1.0,0.1));
		contentPane.add(MergeTrsh);
		
		JLabel label = new JLabel("Used localization:");
		label.setToolTipText("<html>\r\nThis value is the name of the localization<br>\r\n\"locale\" to use for IO operations, it is important<br>\r\nin file encoding, and handling of national<br>\r\ncharacters. If you want to use your system's<br>\r\ndefault leave this field blank. For all possibilites<br>\r\nrun \"locale -a\" command from a system shell.\r\n</html>");
		label.setFont(new Font("Tahoma", Font.PLAIN, 11));
		label.setBounds(10, 115, 117, 14);
		contentPane.add(label);
		
		LocalStr = new JTextField();
		LocalStr.setBounds(112, 112, 267, 20);
		contentPane.add(LocalStr);
		LocalStr.setColumns(10);
		
		JLabel label_1 = new JLabel("Log header part's length:");
		label_1.setToolTipText("<html>\r\nThis value sets the length<br>\r\nof the header part of log messages.<br>\r\nIt is counted in the number of columns\r\n</html>");
		label_1.setFont(new Font("Tahoma", Font.PLAIN, 11));
		label_1.setBounds(10, 78, 148, 14);
		contentPane.add(label_1);
		
		HeaderLen = new JSpinner();
		HeaderLen.setModel(new SpinnerNumberModel(new Integer(4), new Integer(0), null, new Integer(1)));
		HeaderLen.setBounds(186, 73, 40, 20);
		contentPane.add(HeaderLen);
		
		JLabel lblDatabaseFilePath = new JLabel("Database file path: ");
		lblDatabaseFilePath.setToolTipText("<html>\nThis is the path for output/input database file.<br>\nThis value must be set.\n</html>");
		lblDatabaseFilePath.setFont(new Font("Dialog", Font.PLAIN, 11));
		lblDatabaseFilePath.setBounds(10, 146, 117, 14);
		contentPane.add(lblDatabaseFilePath);
		
		DBPath = new JTextField();
		DBPath.setBounds(112, 143, 267, 20);
		contentPane.add(DBPath);
		DBPath.setColumns(10);
		
		JLabel lblNewLabel = new JLabel("Log file's path: ");
		lblNewLabel.setToolTipText("<html>\nThis is the path for log file, this should<br>\nbe set, as online algorithm is usually run<br>\nwithout console, thus console error reporting<br>\nis impossible. Would you leave this field blank<br>\nconsole error output will be used.\n</html>");
		lblNewLabel.setFont(new Font("Dialog", Font.PLAIN, 11));
		lblNewLabel.setBounds(10, 172, 97, 15);
		contentPane.add(lblNewLabel);
		
		LogPath = new JTextField();
		LogPath.setBounds(112, 169, 267, 19);
		contentPane.add(LogPath);
		LogPath.setColumns(10);
		
		RegExpr = new JTextField();
		RegExpr.setBounds(169, 199, 211, 20);
		contentPane.add(RegExpr);
		RegExpr.setColumns(10);
		
		JLabel lblTokenizingRegularExpression = new JLabel("Tokenizing regular expression: ");
		lblTokenizingRegularExpression.setFont(new Font("Tahoma", Font.PLAIN, 11));
		lblTokenizingRegularExpression.setToolTipText("<html>\r\nThis regular expression should match all the separators <br>\r\nin the log messages that separate tokens.<br>\r\nIt must be defined in POSIX regex format\r\n</html>");
		lblTokenizingRegularExpression.setBounds(10, 202, 154, 14);
		contentPane.add(lblTokenizingRegularExpression);
	}
	
	private Node getChildByName(Node n,String name) throws Exception{
		if(n==null || n.getNodeType()!=Node.ELEMENT_NODE){
			JOptionPane.showMessageDialog(contentPane,"The XML file is not valid!","Error",
					JOptionPane.ERROR_MESSAGE);
			throw new Exception();
		}
		
		Element tmpElem=(Element)n;
		return tmpElem.getElementsByTagName(name).item(0);
	}
	
	private String getAttribute(Node n,String AttributeName) throws Exception{
		if(n==null || n.getNodeType()!=Node.ELEMENT_NODE){
			JOptionPane.showMessageDialog(contentPane,"The XML file is not valid!","Error",
					JOptionPane.ERROR_MESSAGE);
			throw new Exception();
		}
		
		Element tmpElem=(Element)n;
		return tmpElem.getAttribute(AttributeName);
	}
	
	private void SaveFile() throws Exception{
		if(OpenedFile==null){
			JFileChooser chooser=new JFileChooser();
			chooser.setFileFilter(new FileNameExtensionFilter("XML file","xml"));
			int ret=chooser.showSaveDialog(contentPane);
			if(ret==JFileChooser.APPROVE_OPTION){
				OpenedFile=chooser.getSelectedFile();
			}
			else{
				return;
			}
		}
		
		DocumentBuilderFactory docFactory=DocumentBuilderFactory.newInstance();
		DocumentBuilder docBuilder=docFactory.newDocumentBuilder();
		Document doc=docBuilder.newDocument();
		Element root=doc.createElement(onlineTag);
		doc.appendChild(root);
		
		Element merge_n=doc.createElement(mergeTag);
		merge_n.setAttribute("value", MergeTrsh.getValue().toString());
		root.appendChild(merge_n);
		Element local_n=doc.createElement(locTag);
		local_n.setAttribute("value", LocalStr.getText());
		root.appendChild(local_n);
		Element length_n=doc.createElement(HeaderTag);
		length_n.setAttribute("value", HeaderLen.getValue().toString());
		root.appendChild(length_n);
		Element portNode=doc.createElement(portTag);
		portNode.setAttribute("value",PortNo.getValue().toString());
		root.appendChild(portNode);
		if(DBPath.getText().isEmpty()){
			JOptionPane.showMessageDialog(contentPane,"The database file's path must be set (must not be empty)!",
					"Error",JOptionPane.ERROR_MESSAGE);
			return;
		}
		Element dbpath_n=doc.createElement(DBPathTag);
		dbpath_n.setAttribute("value",DBPath.getText());
		root.appendChild(dbpath_n);
		Element logpath_n=doc.createElement(LogPathTag);
		logpath_n.setAttribute("value",LogPath.getText());
		root.appendChild(logpath_n);
		String regex=RegExpr.getText();
		try{
			Pattern.compile(regex);
		}
		catch(PatternSyntaxException e){
			JOptionPane.showMessageDialog(contentPane,"The provided regular expression is invalid!\n Did you use POSIX format?",
					"Syntax error",JOptionPane.ERROR_MESSAGE);
			return;
		}
		Element regex_n=doc.createElement(RegexTag);
		regex_n.setAttribute("value",regex);
		root.appendChild(regex_n);
		
		TransformerFactory trfact=TransformerFactory.newInstance();
		Transformer trf=trfact.newTransformer();
		trf.setOutputProperty(OutputKeys.INDENT, "yes");
		trf.setOutputProperty("{http://xml.apache.org/xslt}indent-amount", "4");
		DOMSource src=new DOMSource(doc);
		
		StreamResult file=new StreamResult(OpenedFile);
		trf.transform(src, file);
		setTitle(WindowTitle+" - "+OpenedFile.getName());
	}
}
